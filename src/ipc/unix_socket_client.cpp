#include "unix_socket_client.h"
#include "protocol.h"

#include <chrono>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// Constructor: Initialize socket path and store reference to the ClientData cache
UnixSocketClient::UnixSocketClient(ClientData& clientData, const std::string& socketPath)
    : socketPath_(socketPath), clientData_(clientData) {}

// Destructor: Automatically call stop() to clean up the thread and close the socket safely following RAII
UnixSocketClient::~UnixSocketClient() {
    stop();
}

// Start client method: set the running flag and launch the background socketThread_
bool UnixSocketClient::start() {
    // If already running, do not restart
    if(running_.load()) {
        return true;
    }
    // Set the loop control flag - Set running_ = true before creating the thread to avoid a race condition when entering run()
    running_.store(true);

    // Initialize std::thread to run the private member method UnixSocketClient::run on 'this' object
    socketThread_ = std::thread(&UnixSocketClient::run, this);
    return true;
}

// Safely stop the background communication thread (Graceful Shutdown)
void UnixSocketClient::stop() {
    // Lower running_ flag to false; if already stopped, join the thread and return
    if(!running_.exchange(false)) {
        if(socketThread_.joinable()) {
            socketThread_.join();
        }
        return;
    }

    // Lock mutex to protect the file descriptor and proactively call shutdown() to immediately wake up any blocked recv() or connect() in run()
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        if(socketFd_ >= 0) {
            shutdown(socketFd_, SHUT_RDWR);
        }
    }

    // Wait for the background worker thread to finish completely before reclaiming thread resources
    if(socketThread_.joinable()) {
        socketThread_.join();
    }

    // Completely close the File Descriptor after the background thread has stopped
    closeSocket();
    std::cout << "[Client] Socket thread stopped safely.\n";
}

// Check the running status of the Client thread
bool UnixSocketClient::isRunning() const {
    return running_.load();
}

// Establish a connection to the Monitor Service via Unix Domain Socket
bool UnixSocketClient::connectToServer() {

    // Request the OS to allocate an AF_UNIX Socket File Descriptor
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd < 0) {
        perror("[Client Error] Failed to create socket");
        return false;
    }

    // Initialize the Unix Domain Socket address structure
    struct sockaddr_un addr{};
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    // Call connect() to connect to the Service's socket file
    // If connection fails, close the newly created fd
    if (connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0){
        perror("[Client Error] Failed to connect to Server (Monitor Service not running?)");
        close(fd);
        return false;
    }

    // Store the valid File Descriptor in the member variable under mutex protection
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        socketFd_ = fd;
    }

     std::cout << "[Client] Connected to Server at: " << socketPath_ << "\n";
     return true;
}

// Safely close the File Descriptor
void UnixSocketClient::closeSocket() {
    std::lock_guard<std::mutex> lock(fdMutex_);
    if (socketFd_ >= 0) {
        close(socketFd_);
        socketFd_ = -1;
    }
}

// Main loop running on the background thread following a periodic Request / Response model
void UnixSocketClient::run() {
    while(running_.load()) {
        // Connection loop: If not yet connected to the Service, retry every second
        if(!connectToServer()) {
            for (int i = 0; i < 10  && running_.load(); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            continue;
        }

        // Request/Response communication loop once connected successfully
        while(running_.load()) {
            // Step 1: Create JSON Request message containing {"type":"get_status"}
            std::string request = ipc ::Protocol ::CreateGetStatusRequest();

            // Step 2: Send Request message to the Service via socket
            if(!ipc::Protocol::SendMessage(socketFd_, request)) {
                std::cerr << "[Client Error] Connection lost while sending Request.\n";
                break;          // Exit loop if sending fails
            }

            // Step 3: Wait to receive Response message from the Service (Blocking data read)
            std::string response;
            if(!ipc::Protocol::ReceiveMessage(socketFd_, response)){
                std::cerr << "[Client] Server closed the connection.\n";
                break;          // Exit loop when Server disconnects
            }

            // Step 4: Decode JSON Response string into StatusPayload struct and update ClientData
            ipc::StatusPayload parsedStatus{};
            if(ipc::Protocol::ParseStatusRespone(response, parsedStatus)) {
                // Safely update data into cache for Main Thread to draw UI
                clientData_.update(parsedStatus);
            } else {
                std::cerr << "[Client Warning] Failed to parse JSON Response.\n";
            }

            // Step 5: Sleep for 1 second before sending the next request (split into 10 ticks of 100ms for fast stop responsiveness)
            for (int i = 0; i < 10 && running_.load(); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        // Close socket upon disconnection to prepare for reconnection in the outer loop
        closeSocket();
    }
}

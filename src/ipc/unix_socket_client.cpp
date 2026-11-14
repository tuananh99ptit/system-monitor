#include "unix_socket_client.h"
#include "protocol.h"

#include <chrono>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// Constructor: Initialize the clientData_ reference and store socketPath_
UnixSocketClient::UnixSocketClient(ClientData& clientData, const std::string& socketPath)
    : socketPath_(socketPath), clientData_(clientData) {}

// Destructor: Ensure the background thread is always stopped and joined before destroying the object
UnixSocketClient::~UnixSocketClient() {
    stop();
}

// Start the background communication thread
bool UnixSocketClient::start() {
    // If the thread is already running, do not spawn a duplicate thread
    if(running_.load()) {
        return true;
    }
    running_.store(true);   // Set running_ = true before spawning the thread to prevent a race condition upon entering run()

    // Create a std::thread running the private member function UnixSocketClient::run on 'this' instance
    socketThread_ = std::thread(&UnixSocketClient::run, this);
    return true;
}

// Stop the background communication thread safely (Graceful Shutdown)
void UnixSocketClient::stop() {
    // Lower the running_ flag to false; if it was already false, exit early
    if(!running_.exchange(false)) {
        // Still check joinable() in case run() exited early due to a connection error before stop() was called
        if(socketThread_.joinable()) {
            socketThread_.join();
        }
        return;
    }

    // Proactively call shutdown() to immediately wake up recv() or connect() if blocked inside run()
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        if(socketFd_ >= 0) {
            shutdown(socketFd_, SHUT_RDWR);
        }
    }

    // Wait for the worker thread to finish completely before reclaiming thread resources
    if(socketThread_.joinable()) {
        socketThread_.join();
    }

    // Fully close the File Descriptor after the worker thread has stopped
    closeSocket();
    std::cout << "[Client] Socket thread stopped safely.\n";
}

// Check the active status of the Client thread
bool UnixSocketClient::isRunning() const {
    return running_.load();
}

// Establish connection to the Monitor Service via Unix Domain Socket
bool UnixSocketClient::connectToServer() {
    // Request the OS to allocate an AF_UNIX Socket File Descriptor
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd < 0) {
        perror("[Client Error] Failed to create socket");
        return false;
    }

    // Configure the target socket file address
    struct sockaddr_un addr{};
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    // Connect to the Server
    if (connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr))) {
        perror("[Client Error] Failed to connect to Server (Is Monitor Service running?)");
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
    if(socketFd_ >= 0) {
        close(socketFd_);
        socketFd_ = -1;
    }
}

// Main communication loop running on the background worker thread (socketThread_)
void UnixSocketClient::run() {
    // Step 1: Establish connection to the Server; if it fails, lower running_ and exit the thread
    if (!connectToServer()) {
        running_.store(false);
        return;
    }

    // Step 2: Periodic request-response loop every second while the running_ flag remains active
    while (running_.load()) {
        // 2.1. Create the JSON Request message: {"type":"get_status"}
        std::string request = ipc::Protocol::CreateGetStatusRequest();

        // 2.2. Send the framed Request message over the Socket
        if (!ipc::Protocol::SendMessage(socketFd_, request)) {
            if (running_.load()) {
                std::cerr << "[Client Error] Connection lost while sending Request to Server.\n";
            }    
            break; // Exit loop if sending fails
        }

        // 2.3. Block and wait to receive the complete JSON Response message frame from the Server
        std::string response;
        if (!ipc::Protocol::ReceiveMessage(socketFd_, response)) {
            if (running_.load()) {
                std::cerr << "[Client] Server closed the connection or data receive error occurred.\n";
            }
            break; // Exit loop when Server disconnects
        }

        // 2.4. Deserialize the JSON Response string into StatusPayload and update ClientData
        ipc::StatusPayload parsedStatus{};
        if (ipc::Protocol::ParseStatusRespone(response, parsedStatus)) {
            clientData_.update(parsedStatus); // Thread-safe write via mutex for the Main Thread to read
        } else {
            std::cerr << "[Client Warning] Failed to parse JSON Response: " << response << "\n";
        }

        // 2.5. Sleep for 1 second between samples, split into ten 100ms intervals for fast exit when stop() is called
        for (int i = 0; i < 10 && running_.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } // <--- Closing brace of while(running_.load()) here

    // Step 3: Clean up the socket OUTSIDE the loop before completely terminating the worker thread
    closeSocket();
    running_.store(false);
}

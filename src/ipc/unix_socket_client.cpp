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

void UnixSocketClient::stop() {
    // 1. Lower the running_ flag to false
    if (!running_.exchange(false)) {
        if (socketThread_.joinable()) {
            socketThread_.join();
        }
        return;
    }

    // 2. IMPORTANT: Call shutdown() to break the blocking recv() call in the background thread
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        if (socketFd_ >= 0) {
            shutdown(socketFd_, SHUT_RDWR); // Immediately wake up the blocking recv() or connect() call
        }
    }

    // 3. Wait for the background thread to finish safely
    if (socketThread_.joinable()) {
        socketThread_.join();
    }

    // 4. Completely close the socket
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

// Main communication loop running on the background worker thread (socketThread_) using the Subscriber model
void UnixSocketClient::run() {
    // Step 1: Establish connection to the Server; if it fails, lower running_ and exit the thread
    if (!connectToServer()) {
        running_.store(false);
        return;
    }

    // Step 2: Loop to listen for data proactively pushed by the Server (Server Push)
    while (running_.load()) {
        // 2.1. Block and wait to receive the complete JSON message frame pushed from the Server
        std::string response;
        if (!ipc::Protocol::ReceiveMessage(socketFd_, response)) {
            if (running_.load()) {
                std::cerr << "[Client] Server closed the connection or data receive error occurred.\n";
            }
            break; // Exit the loop when the Server disconnects
        }

        // 2.2. Deserialize the received JSON string into a StatusPayload struct and update ClientData
        ipc::StatusPayload parsedStatus{};
        if (ipc::Protocol::ParseStatusRespone(response, parsedStatus)) {
            clientData_.update(parsedStatus); // Thread-safe write via mutex for the Main Thread to render the UI
        } else {
            std::cerr << "[Client Warning] Failed to parse JSON Response: " << response << "\n";
        }
    }

    // Step 3: Clean up the socket outside the loop before terminating the worker thread
    closeSocket();
    running_.store(false);
}

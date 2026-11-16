#include "unix_socket_server.h"
#include "protocol.h"

#include <cstring>
#include<iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// Constructor: Initialize socket path and store reference to the MonitorData store
UnixSocketServer::UnixSocketServer(MonitorData& monitorData, const std::string& socketPath):socketPath_(socketPath), monitorData_(monitorData) {}

// Destructor: Automatically call stop() to safely clean up resources following RAII when the object is destroyed
UnixSocketServer::~UnixSocketServer() {
    stop();
}

// Initialize the server-side listening socket
bool UnixSocketServer::initServerSocket() {
    // Create a stream socket file descriptor using the Unix address family (AF_UNIX)
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd < 0) {
         perror("[Server Error] socket() failed");
         return false;
    }

    // Remove the old socket file on disk if the previous process terminated abruptly
    unlink(socketPath_.c_str());

    // Initialize the Unix Domain Socket address structure
    struct sockaddr_un addr{};
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    // Bind the socket to the file path /tmp/system_monitor.sock
    if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("[Server Error] bind() failed");
        close(fd);
        return false;
    }
    
    // Put the socket into listening state for Client connections (max 5 pending connections / Backlog = 5)
    if (listen(fd, 5) < 0) {
        perror("[Server Error] listen() failed");
        close(fd);
        unlink(socketPath_.c_str());
        return false;
    }

    // Store serverFd_ under mutex protection
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        serverFd_ = fd;
    }
    std::cout << "[Server] Unix Socket Server is listening at: " << socketPath_ << "\n";
    return true;
}

// Start the server: call initServerSocket and spawn the background socketThread_
bool UnixSocketServer::start() {
    // Avoid duplicate initialization if the Server is already running
    if(running_.load()) {
        return true;
    }
    // If socket initialization fails, return false
    if(!initServerSocket()) {
        return false;
    }
    running_.store(true);
    // Create a background thread executing the run() method of the current object
    socketThread_ = std::thread(&UnixSocketServer::run, this);
    return true;
}

// Safely stop the Server (Graceful Shutdown)
void UnixSocketServer::stop() {
    // Lower the running_ flag to false; if already stopped, just check joinable and return
    if(!running_.exchange(false)) {
        if(socketThread_.joinable()) {
            socketThread_.join();
        }
        return ;
    }

    // Call shutdown() on both clientFd_ and serverFd_ to immediately wake up
    // blocking calls such as accept() or send() running inside socketThread_
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        if (clientFd_ >= 0) {
            shutdown(clientFd_, SHUT_RDWR);
        }
        if (serverFd_ >= 0) {
            shutdown(serverFd_, SHUT_RDWR);
        }
    }

    // Wait for the background socketThread_ to finish completely
    if (socketThread_.joinable()) {
        socketThread_.join();
    }

    // Close the File Descriptors and delete the virtual socket file on disk
    closeSockets();
    unlink(socketPath_.c_str());
    std::cout << "[Server] Unix Socket Server stopped safely.\n";
}

// Check the running status of the server
bool UnixSocketServer::isRunning() const {
    return running_.load();
}

// Dedicated function to handle the Request / Response model with a specific client connection
void UnixSocketServer::handleClient(int clientFd) {
    while(running_.load()) {
        std::string requestJson;

        // Receive Request message from Client (Blocking data read over socket)
        if(!ipc::Protocol::ReceiveMessage(clientFd, requestJson)){
            break; // Exit loop if client disconnects or a transmission error occurs
        }

        // Parse the command type from the received JSON Request string
        std::string reqType = ipc::Protocol::ParseRequestType(requestJson);
        if(reqType == "get_status") {
            // Retrieve the latest hardware metrics from MonitorData store (updated by Worker Thread)
            auto data = monitorData_.Get();

            // Convert data to the StatusPayload structure of the Protocol layer
            ipc::StatusPayload payload{};
            payload.cpu = data.cpuUsage;
            payload.memory = data.memoryUsage;
            payload.disk = data.diskUsage;
            payload.processCount = data.processCount;

            // Create JSON Response string and send it back to the Client via socket
            std::string responseJson = ipc::Protocol::CreateStatusResponse(payload);
            if(!ipc::Protocol::SendMessage(clientFd, responseJson)){
                break;      // Exit loop if sending fails
            }
        }
    }
}

// Safely close clientFd_ and serverFd_ under mutex protection
void UnixSocketServer::closeSockets() {
    std::lock_guard<std::mutex> lock(fdMutex_);
    if (clientFd_ >= 0) {
        close(clientFd_);
        clientFd_ = -1;
    }
    if (serverFd_ >= 0) {
        close(serverFd_);
        serverFd_ = -1;
    }
}

// Main loop running on the background thread to wait for Client connections
void UnixSocketServer::run() {
    while(running_.load()) {
        std::cout << "[Server] Waiting for Client connection...\n";
        // accept() is a Blocking Call: waits indefinitely until a Client connects
        int acceptedFd = accept(serverFd_, nullptr, nullptr);
        if(acceptedFd < 0) {
            if(!running_.load()) {
                break;  // Exit loop if the server is shutting down and accept was interrupted
            }
            continue;
        }

        // Store the current clientFd_ under mutex protection
        {
            std::lock_guard<std::mutex> lock(fdMutex_);
            clientFd_ = acceptedFd;
        }
        std::cout << "[Server] Client connected (clientFd = " << acceptedFd << ")\n";

        // Continuously serve the Request/Response model for this client until they disconnect
        handleClient(acceptedFd);

        // Clean up clientFd_ after the client disconnects
        {
            std::lock_guard<std::mutex> lock(fdMutex_);
            if (clientFd_ >= 0) {
                close(clientFd_);
                clientFd_ = -1;
            }
        }
        std::cout << "[Server] Client disconnected. Ready for a new client.\n";
    }
}

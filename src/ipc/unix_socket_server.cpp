#include "unix_socket_server.h"
#include "protocol.h"   // Uses ipc::Protocol to create JSON Responses and frame messages
#include <chrono>       // Provides std::chrono::milliseconds
#include <cstring>      // Provides std::memset and std::strncpy
#include <iostream>     // Provides std::cout and std::cerr
#include <sys/socket.h> // Provides socket(), bind(), listen(), accept(), shutdown()
#include <sys/un.h>     // Provides the sockaddr_un address structure
#include <unistd.h>     // Provides close() and unlink()

// Constructor: Store the socket path and reference to the MonitorData storage
UnixSocketServer::UnixSocketServer(MonitorData& monitorData, const std::string& socketPath)
    : socketPath_(socketPath), monitorData_(monitorData) {}

// Destructor: Ensure the background thread and sockets are always cleaned up before destroying the object
UnixSocketServer::~UnixSocketServer() {
    stop();
}

// Set up the listening socket for the Server
bool UnixSocketServer::initServerSocket() {
    // 1. Request the Linux kernel to allocate a stream-oriented Unix Domain Socket (SOCK_STREAM)
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("[Server Error] Failed to create socket");
        return false;
    }

    // 2. Remove any leftover socket file from a previous run to avoid the "Address already in use" error
    unlink(socketPath_.c_str());

    // 3. Populate the socket file address structure
    struct sockaddr_un addr{};
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    // 4. Bind the socket to the file path /tmp/system_monitor.sock
    if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("[Server Error] Failed to bind socket");
        close(fd);
        return false;
    }

    // 5. Open the listening queue for incoming Client connections (up to 5 pending connections)
    if (listen(fd, 5) < 0) {
        perror("[Server Error] Listen failed");
        close(fd);
        unlink(socketPath_.c_str());
        return false;
    }

    // Save the valid serverFd_ under mutex protection
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        serverFd_ = fd;
    }

    std::cout << "[Server] Unix Socket Server is listening at: " << socketPath_ << "\n";
    return true;
}

// Start the Server and spawn the background communication thread (socketThread_)
bool UnixSocketServer::start() {
    // Avoid duplicate initialization if the Server is already running
    if (running_.load()) {
        return true;
    }

    // Initialize and open the listening socket before starting the worker thread
    if (!initServerSocket()) {
        return false;
    }

    running_.store(true); // Set the running flag before spawning the thread

    // Create a std::thread executing UnixSocketServer::run on the current instance
    socketThread_ = std::thread(&UnixSocketServer::run, this);
    return true;
}

// Stop the Server safely (Graceful Shutdown)
void UnixSocketServer::stop() {
    // Lower the running_ flag to false; if already stopped previously, simply check joinable and return
    if (!running_.exchange(false)) {
        if (socketThread_.joinable()) {
            socketThread_.join();
        }
        return;
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

    // Wait for the worker thread to finish execution and terminate completely
    if (socketThread_.joinable()) {
        socketThread_.join();
    }

    // Close the File Descriptors and remove the virtual socket file from disk
    closeSockets();
    unlink(socketPath_.c_str());
    std::cout << "[Server] Unix Socket Server stopped safely.\n";
}

// Check the active status of the Server
bool UnixSocketServer::isRunning() const {
    return running_.load();
}

// Safely close clientFd_ and serverFd_
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

// Main loop of the Publisher thread running on socketThread_
void UnixSocketServer::run() {
    // Outer loop allows the Server to accept a new Client if the previous Client disconnects
    while (running_.load()) {
        std::cout << "[Server] Waiting for a Client to subscribe for data (Subscriber)...\n";

        // 1. Block and wait until a Client connects
        int acceptedFd = accept(serverFd_, nullptr, nullptr);
        if (acceptedFd < 0) {
            // If accept() returns an error because stop() closed serverFd_, exit the loop cleanly
            if (!running_.load()) {
                break;
            }
            perror("[Server Error] Accept failed");
            continue;
        }

        // Store the current clientFd_ under mutex protection
        {
            std::lock_guard<std::mutex> lock(fdMutex_);
            clientFd_ = acceptedFd;
        }

        std::cout << "[Server] Client connected! Starting Server Push mode (clientFd = "
                  << acceptedFd << ")\n";

        // 2. Publish loop (Server Push): Proactively push data periodically every 1 second
        while (running_.load()) {
            // 2.1. Retrieve a copy of the latest system metrics from MonitorData (updated by MonitorWorker)
            auto currentMetrics = monitorData_.Get();

            // 2.2. Pack the metrics into the Protocol layer's StatusPayload struct
            ipc::StatusPayload payload{};
            payload.cpu = currentMetrics.cpuUsage;
            payload.memory = currentMetrics.memoryUsage;
            payload.disk = currentMetrics.diskUsage;
            payload.processCount = currentMetrics.processCount;

            // 2.3. Serialize StatusPayload into a standard JSON string
            std::string jsonResponse = ipc::Protocol::CreateStatusResponse(payload);

            // 2.4. Proactively push the framed message [4B Length | JSON] down to the Client
            if (!ipc::Protocol::SendMessage(acceptedFd, jsonResponse)) {
                if (running_.load()) {
                    std::cout << "[Server] Client (fd=" << acceptedFd
                              << ") disconnected. Stopping push.\n";
                }
                break; // Exit the push loop to close clientFd_ and return to waiting for a new Client
            }

            // 2.5. Sleep for 1 second between broadcasts (split into 100ms x 10 for quick exit when stop() is called)
            for (int i = 0; i < 10 && running_.load(); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        // Close the connection with the served Client before returning to the accept() loop
        {
            std::lock_guard<std::mutex> lock(fdMutex_);
            if (clientFd_ >= 0) {
                close(clientFd_);
                clientFd_ = -1;
            }
        }
    }
}

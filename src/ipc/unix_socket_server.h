#ifndef UNIX_SOCKET_SERVER_H
#define UNIX_SOCKET_SERVER_H

#include "monitor_data.h"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>

// Declare the IPC Server management class on the monitor_service process side
class UnixSocketServer {
public:
    // Constructor: Receive a reference to the MonitorData store and the socket file path (default "/tmp/system_monitor.sock")
    explicit UnixSocketServer(MonitorData& monitorData, const std::string& socketPath = "/tmp/system_monitor.sock");

    // Destructor: Following RAII, automatically call stop() to clean up thread and socket resources upon object destruction
    ~UnixSocketServer();

    // Disable Copy Constructor to prevent duplicating objects holding exclusive OS resources
    UnixSocketServer(const UnixSocketServer&) = delete;
    // Disable Copy Assignment Operator to prevent assigning data between Server objects
    UnixSocketServer& operator=(const UnixSocketServer&) = delete;

    // Start the server: create socket, bind, listen, and spawn the background socketThread_ running the run() method
    bool start();

    // Safely stop the server (Graceful Shutdown): wake up blocking calls, join thread, close sockets, and delete the virtual file
    void stop();

    // Const method to quickly check whether the server is currently running
    bool isRunning() const;

private:
    // Core function running on the background thread: executes the accept() loop to accept incoming Client connections
    void run();

    // Dedicated function to serve Request/Response for a specific Client after connection is established
    void handleClient(int clientFd);

    // Internal function to initialize the server socket: calls socket(), unlink() old file, bind(), and listen()
    bool initServerSocket();

    // Internal function to safely close the serverFd_ and clientFd_ file descriptors under mutex protection
    void closeSockets();

private:
    std::string socketPath_;            // Variable storing the Unix Domain Socket file path string on the system
    int serverFd_{-1};                  // File Descriptor of the listening socket (initialized to -1 meaning not open)
    int clientFd_{-1};                  // File Descriptor of the socket connected to the current Client
    std::mutex fdMutex_;                // Mutex lock synchronizing access to the socket file descriptors
    std::thread socketThread_;          // std::thread object managing the background thread accepting network connections
    std::atomic<bool> running_{false};  // Atomic flag controlling the run/stop lifecycle of the server (default false)
    MonitorData& monitorData_;          // Reference to the hardware data store updated by the Worker Thread
};

#endif // UNIX_SOCKET_SERVER_H

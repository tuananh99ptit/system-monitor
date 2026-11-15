#ifndef UNIX_SOCKET_SERVER_H
#define UNIX_SOCKET_SERVER_H

#include "monitor_data.h"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>

// Class managing the Server-side IPC connection using the Publish / Subscribe (Server Push) pattern
class UnixSocketServer {
public:
    // Constructor: Receives a reference to the shared MonitorData and the socket file path
    explicit UnixSocketServer(MonitorData& MonitorData, const std::string& socketPath = "/tmp/system_monitor.sock");

    // Destructor: Automatically calls stop() to terminate the background thread and clean up sockets per RAII
    ~UnixSocketServer();

    // Disable Copy Constructor and Copy Assignment since the class owns a std::thread and File Descriptors
    UnixSocketServer(const UnixSocketServer&) = delete;
    UnixSocketServer& operator=(const UnixSocketServer&) = delete;

    // Initialize the listening socket and start the Publisher thread (socketThread_)
    bool start();

    // Stop the Publisher thread, wake up blocking calls, join the thread, and remove the socket file
    void stop();

    // Check the active status of the Server
    bool isRunning() const;

private:

    // Main execution routine of the background thread: Accept Client -> Push JSON every 1 second
    void run();

    // Initialize serverFd_, bind to the file path, and switch to listen() state
    bool initServerSocket();

    // Safely close both clientFd_ and serverFd_
    void closeSockets();

private:

    // Path to the socket file (/tmp/system_monitor.sock)
    std::string socketPath_;

    // Socket File Descriptor listening for incoming connections
    int serverFd_{-1};

    // Socket File Descriptor of the currently connected Client
    int clientFd_{-1};

    // Mutex protecting shutdown/close operations on serverFd_ and clientFd_
    std::mutex fdMutex_;

    // Worker thread dedicated to accepting connections and pushing data to the Client
    std::thread socketThread_;

    // Atomic flag controlling the thread lifecycle
    std::atomic<bool> running_{false};

    // Reference to the data storage updated by MonitorWorker
    MonitorData& monitorData_;
};

#endif // UNIX_SOCKET_SERVER_H

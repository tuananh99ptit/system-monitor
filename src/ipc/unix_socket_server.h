#ifndef UNIX_SOCKET_SERVER_H
#define UNIX_SOCKET_SERVER_H

#include <string>
#include <atomic>

// Class managing the IPC Server on the Monitor Service side using Unix Domain Socket
class UnixSocketServer {
public:
    // By default, creates the socket file at /tmp/system_monitor.sock
    explicit UnixSocketServer(const std::string& socketPath = "/tmp/system_monitor.sock");
    
    // Destructor automatically cleans up the socket and removes the socket file (RAII)
    ~UnixSocketServer();

    // Disable Copy Constructor and Copy Assignment Operator
    UnixSocketServer(const UnixSocketServer&) = delete;
    UnixSocketServer& operator=(const UnixSocketServer&) = delete;

    // Initialize socket, unlink old file, bind, and listen
    bool Start();

    // Loop to listen for and accept incoming connections from Clients (Blocking call)
    void ListenLoop();

    // Safely stop the Server and close file descriptors
    void Stop();

private:
    void CleanUp();

    std::string socketPath_;
    int serverFd_{-1};
    std::atomic<bool> running_{false};
};

#endif // UNIX_SOCKET_SERVER_H

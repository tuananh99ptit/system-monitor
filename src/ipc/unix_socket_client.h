#ifndef UNIX_SOCKET_CLIENT_H
#define UNIX_SOCKET_CLIENT_H

#include "client_data.h"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>

// Class managing the Client-side IPC connection using the "Object Owns Thread" pattern
class UnixSocketClient {
public:
    // Constructor: Receives a reference to the shared ClientData and the path to the socket file
    explicit UnixSocketClient(ClientData& clientData, const std::string& socketPath = "/tmp/system_monitor.sock");

    // Destructor: Automatically calls stop() to terminate the background thread and safely close the socket (RAII)
    ~UnixSocketClient();

    // Disable Copy Constructor and Copy Assignment since the class owns a std::thread and File Descriptor
    UnixSocketClient(const UnixSocketClient&) = delete;
    UnixSocketClient& operator=(const UnixSocketClient&) = delete;

    // Starts the background communication thread (socketThread_)
    bool start();

    // Stops the background communication thread, closes the socket, and reclaims the thread via join()
    void stop();

    // Checks whether the communication thread is currently active
    bool isRunning() const;

private:
    // Main execution routine of the background thread: Connect -> Send Request -> Receive Response -> Update ClientData
    void run();

    // Internal helper to establish the Unix Domain Socket connection to the Server
    bool connectToServer();

    // Internal helper to safely close the File Descriptor
    void closeSocket();

private:
    // Path to the virtual socket file (/tmp/system_monitor.sock)
    std::string socketPath_;

    // Client's Socket File Descriptor (-1 means unopened)
    int socketFd_{-1};

    // Mutex protecting shutdown/close operations on socketFd_ between two threads
    std::mutex fdMutex_;

    // Worker thread object dedicated to IPC communication
    std::thread socketThread_;

    // Atomic flag controlling the thread's lifecycle loop
    std::atomic<bool> running_{false};

    // Reference to the data storage shared with the Main Thread (UI)
    ClientData& clientData_;
};

#endif // UNIX_SOCKET_CLIENT_H

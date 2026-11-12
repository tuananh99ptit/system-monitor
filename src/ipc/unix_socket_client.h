#ifndef UNIX_SOCKET_CLIENT_H
#define UNIX_SOCKET_CLIENT_H

#include <string>
#include <sys/types.h>

// Class encapsulating and managing the Client-side IPC connection using Unix Domain Socket (AF_UNIX)
class UnixSocketClient {
public:
    // Constructor: Receives the socket file path (defaults to /tmp/system_monitor.sock)
    // The explicit keyword prevents the compiler from performing implicit conversion from std::string
    explicit UnixSocketClient(const std::string& socketPath = "/tmp/system_monitor.sock");

    // Destructor: Automatically calls Disconnect() to close the socket and release the File Descriptor (RAII design standard)
    ~UnixSocketClient();

    // Disable Copy Constructor and Copy Assignment Operator using the = delete keyword
    // Reason: The class manages a unique system resource, the File Descriptor (clientFd_); copying would lead to a double-close error
    UnixSocketClient(const UnixSocketClient&) = delete;
    UnixSocketClient& operator=(const UnixSocketClient&) = delete;

    // Executes socket() and connect() system calls to the Server, returning true on success
    bool Connect();

    // Safely disconnects from the Server: shuts down the I/O stream with shutdown() and closes clientFd_
    void Disconnect();

    // Const getter method: Checks whether the Client currently holds a valid connection to the Server
    bool IsConnected() const;

    // Sends text string data (or JSON/Payload strings) to the Server
    bool SendMessage(const std::string& message);

    // Reads data from the Server into the buffer array, returning the actual number of bytes read (or <= 0 on disconnect/error)
    ssize_t ReceiveMessage(char* buffer, size_t bufferSize);

private:
    // Stores the path to the virtual socket file on the system disk (/tmp/system_monitor.sock)
    std::string socketPath_;

    // Client's Socket File Descriptor (default initialized to -1, meaning unopened)
    int clientFd_{-1};

    // Flag tracking the connection state (true: successfully connected, false: disconnected/not connected)
    bool connected_{false};
};

#endif // UNIX_SOCKET_CLIENT_H

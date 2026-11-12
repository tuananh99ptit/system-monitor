#include "unix_socket_client.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

// Constructor: Use Member Initializer List to assign the socket file path to socketPath_
// clientFd_ initialized to -1, connected_ = false (per header declaration)
UnixSocketClient::UnixSocketClient(const std::string& socketPath) : socketPath_(socketPath) {}

// Destructor: Automatically call Disconnect() when the object is destroyed (Ensures RAII standard)
UnixSocketClient::~UnixSocketClient() {
    // Automatically close the socket and release the File Descriptor
    Disconnect();
}

// Function to actively connect to the Unix Socket Server
bool UnixSocketClient::Connect() {
    // If already connected successfully, skip
    if(connected_) return true;

    // 1. Create Socket Endpoint on the Client side:
    // AF_UNIX: Unix Domain socket family (Local IPC)
    // SOCK_STREAM: Use a reliable, connection-oriented byte stream
    // 0: Default protocol
    clientFd_ = socket(AF_UNIX , SOCK_STREAM, 0);
    
    // If return value < 0, the system failed to allocate a socket FD
    if(clientFd_ < 0) {
        perror("[Client Error] Failed to create socket");
        return false;
    }

    // 2. Specify the target Server address to connect to
    // Declare the Unix Domain Socket address struct
    // Zero out the addr memory region to avoid garbage data
    // Assign the address family as AF_UNIX
    struct sockaddr_un addr{};
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    // Safely copy the path string (/tmp/system_monitor.sock) into sun_path
    std::strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    // 3. Execute the connect() System Call: Actively request the Kernel to connect to the Server
    // Cast the struct sockaddr_un* pointer to the standard POSIX C-API struct sockaddr*
    if (connect(clientFd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("[Client Error] Failed to connect to Server (Is the Server running?)");
        close(clientFd_);  // Close the newly opened File Descriptor to prevent resource leaks
        clientFd_ = -1;    // Reset socketFd back to -1
        return false;      // Report connection failure
    }
    
    // Update the connection status flag to successful
    connected_ = true;
    std::cout << "[Client] Successfully connected to Server: " << socketPath_ << std::endl;
    return true;
}

// Function to safely disconnect from the Server (Graceful Disconnect)
void UnixSocketClient::Disconnect() {
    // If not connected, no need to disconnect
    if(!connected_) return;

    if(clientFd_ >= 0) {
        // 1. Shut down both read and write channels (SHUT_RDWR) of the socket to immediately terminate transmission at the Kernel level
        shutdown(clientFd_, SHUT_RDWR);
        
        // 2. Completely close the Client's File Descriptor
        close(clientFd_);
        clientFd_ = -1;
    }

    // Lower the connection status flag
    connected_ = false;
    std::cout << "[Client] Disconnected from Server." << std::endl;
}

// Getter to check connection status
bool UnixSocketClient::IsConnected() const {
    return connected_;
}

// Function to send a message / data string to the Server
bool UnixSocketClient::SendMessage(const std::string& message) {
    // Do not send if there is no valid connection
    if(!connected_ || clientFd_ < 0) return false;

    // send() System Call:
    // MSG_NOSIGNAL: Crucial flag preventing the Kernel from raising a SIGPIPE signal that crashes the process if the Server disconnects abruptly
    ssize_t bytesSent = send(clientFd_, message.c_str(), message.length(), MSG_NOSIGNAL);
    if(bytesSent < 0) {
        perror("[Client Error] Failed to send data");
        return false;
    }
    return true;
}

// Function to receive response data from the Server into the buffer
ssize_t UnixSocketClient::ReceiveMessage(char* buffer, size_t bufferSize) {
    // Return -1 if not connected
    if(!connected_ || clientFd_ < 0) return -1;

    // Clear the buffer memory before reading new data
    std::memset(buffer, 0, bufferSize);

    // recv() System Call: Wait and read up to (bufferSize - 1) bytes, reserving 1 byte for the null-terminator '\0'
    ssize_t bytesRead = recv(clientFd_, buffer, bufferSize - 1 , 0);
    if(bytesRead <= 0) {
        // If bytesRead == 0: Server actively closed the connection (received FIN)
        // If bytesRead < 0: Network/transmission error occurred
        // Mark the Client as disconnected
        connected_ = false;
    }
    // Return the actual number of bytes received from the Server
    return bytesRead;
}

#include "unix_socket_server.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

// Constructor: Use Member Initializer List to assign the passed socketPath to the member variable socketPath_
UnixSocketServer::UnixSocketServer(const std::string& socketPath) : socketPath_(socketPath) {}

// Destructor: Automatically call Stop() when the object is destroyed to ensure resource cleanup (RAII standard)
UnixSocketServer::~UnixSocketServer() {
    // Release the socket file descriptor and remove the socket file from disk
    Stop();
}

// Helper function to close the File Descriptor and clean up the virtual file on disk
void UnixSocketServer::CleanUp(){
    // 2. Close the Server's Socket File Descriptor if it is open (not -1)
    if(serverFd_ >= 0) {
        shutdown(serverFd_, SHUT_RDWR); // Forcefully shut down all read/write channels of the socket to unblock accept() running on a worker thread
        close(serverFd_);               // Close the socket connection at the Kernel level
        serverFd_ = -1;                 // Reset the value to -1 to mark the fd as closed
    }

    // 3. Remove the virtual socket file from the Linux disk filesystem (/tmp/system_monitor.sock)
    unlink(socketPath_.c_str());
}


// Stop the Server safely (Graceful Shutdown)
void UnixSocketServer::Stop() {
    if(!running_) return ;              // If the Server is already stopped, do nothing further
    running_ = false;                   // Set the running_ flag to false to exit while (running_) loops
    CleanUp();                          // Proceed with resource cleanup
    std::cout << "[Server] Unix Socket Server stopped safely." << std::endl;
}


// Function to initialize and start listening for Server connections
bool UnixSocketServer::Start() {

    // If the Server is already running, skip and do not re-initialize
    if(running_) {
        return true;
    }

    // 1. Create Socket Endpoint (Create Socket with AF_UNIX family)
    // AF_UNIX: Socket family dedicated to local IPC on the same Linux machine
    // SOCK_STREAM: Use a reliable data stream (similar to the TCP mechanism)
    // 0: Let the system automatically choose the default protocol
    serverFd_ = socket(AF_UNIX, SOCK_STREAM, 0);

    // If the return value is < 0, initialization failed (out of file descriptors or OS error)
    if(serverFd_ < 0) {
        perror("[Server Error] Failed to create socket");
        return false;
    }

    // 2. Remove the old socket file if it still exists from a previous run (prevents "Address already in use" error)
    unlink(socketPath_.c_str());

    // 3. Declare and initialize the Unix Domain socket address configuration
    // Todo: Declare the Unix Domain Socket address struct
    // Zero out the addr memory region to avoid garbage data
    // Assign the address family as AF_UNIX
    struct sockaddr_un addr{};
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    // Safely copy the path string (e.g., /tmp/system_monitor.sock) into the sun_path variable
    std::strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    // 4. Bind the socket to the file path
    if (bind(serverFd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0){
        perror("[Server Error] Bind failed");
        CleanUp();
        return false;
    }

    // 5. Execute the listen() System Call: Switch the socket to listening state (Passive mode)
    // Parameter 5 (backlog): Maximum number of pending connections in the Kernel's queue
    if(listen(serverFd_, 5) < 0) {
        perror("[Server Error] Listen failed");
        CleanUp();
        return false;
    }

    running_ = true;
    std::cout << "[Server] Unix Socket Server is listening at: " << socketPath_ << std::endl;
    return true;
}

// Loop to continuously accept incoming connections from Clients
void UnixSocketServer::ListenLoop() {
    std::cout << "[Server] Waiting for Client connections..." << std::endl;

    // Loop to accept Clients while the Server is still in the running_ == true state
    while(running_) {
        // accept() is a Blocking System Call (waits at this line until a Client connects)
        // Returns a new File Descriptor (clientFd) representing the dedicated channel to that Client
        int clientFd = accept(serverFd_, nullptr, nullptr);
        // If accept returns a negative value (error or interrupted by a signal)
        if(clientFd < 0) {
            // If the program is being stopped from the Stop() function, exit the loop safely
            if(!running_) break;
            // Print an error if an actual connection issue occurred
            perror("[Server Error] Accept failed");
            // Skip this iteration and continue the loop to wait for the next connection
            continue;
        }
        std::cout << "[Server] Client connected! (clientFd = " << clientFd << ")" << std::endl;
        // Temporarily close clientFd right after accepting (data read/write will be implemented in Task 6 & Task 8)
        close(clientFd);
    }
}

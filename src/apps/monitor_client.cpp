#include "signal_handler.h"
#include "unix_socket_client.h"
#include <chrono>
#include <iostream>
#include <unistd.h>

int main() {
    // 1. Register system interrupt signal handlers (SIGINT - Ctrl+C, SIGTERM) to ensure Graceful Shutdown
    if(!ShutdownSignal::Install()) {
        // Print an error message if the Signal Handler cannot be set up
        std::cerr << "Failed to install signal handlers\n";
        return 1;
    }

    // Print Monitor Client process identification info (PID: current process ID, PPID: parent process ID)
    std::cout << "Monitor Client Started (pid=" << getpid() << ", ppid=" << getppid() << ")" << std::endl;

    // TODO Task 7: Initialize the background data receiving thread and render the UI via ConsoleRenderer (To be implemented in Task 7)

    // 2. Initialize the Client object managing the Unix Domain Socket connecting to /tmp/system_monitor.sock
    UnixSocketClient client("/tmp/system_monitor.sock");

    // 3. Execute the connect() System Call: Actively send a connection request to the Monitor Service process (Server)
    if(!client.Connect()) {
        std::cerr << "[Client] Unable to connect to Monitor Service! Ensure the Service is running.\n";
        return 1;
    }

    // 4. Loop to keep the Client process alive:
    // The loop continues running while NO Ctrl+C interrupt signal has been received (IsRequested == false) AND the Socket connection is still alive (IsConnected == true)
    while(!ShutdownSignal::IsRequested() && client.IsConnected()) {
        // Sleep for 1 second per cycle to avoid 100% CPU usage while checking the Signal flag
        ShutdownSignal::SleepUnlessRequested(std::chrono::seconds(1));
    }

    // Print the reason for exiting the loop (either user pressed Ctrl+C or Server closed the connection)
    std::cout << "\nReceived signal or disconnected: " << ShutdownSignal::LastSignal() << std::endl;

    // 5. Perform a safe disconnect from the Server (calls shutdown(clientFd, SHUT_RDWR) and close(clientFd))
    client.Disconnect();

    std::cout << "Monitor Client stopped." << std::endl;
    return 0;
}

/*
Task 4 : phase 4
Standard 2-Process testing procedure:
Due to the nature of the Client-Server IPC model, the Server must be running concurrently for the Client (nc - Netcat) to connect.
Open Terminal 1 (Service): ./monitor_service (Leave Terminal 1 running in the background, DO NOT press Ctrl+C.)
Open Terminal 2 (Client - Click the + icon or Split Terminal in VS Code): nc -U /tmp/system_monitor.sock
-> output: [Server] Client connected! (clientFd = 4)
*/

#include "signal_handler.h"
#include "unix_socket_server.h" // Add IPC Server Header from Task 4

#include <chrono>
#include <iostream>
#include <thread> // Add thread library to run ListenLoop in the background
#include <unistd.h>

int main() {
    // 1. Register system signal handlers (SIGINT, SIGTERM)
    if (!ShutdownSignal::Install()) {
        std::cerr << "Failed to install signal handlers\n";
        return 1;
    }

    std::cout << "Monitor Service Started (pid=" << getpid() << ", ppid=" << getppid() << ")" << std::endl;

    // TODO Task 8: start MonitorWorker (already implemented in Phase 3)

    // 2. Integrate Task 4: Initialize and start the Unix Socket Server
    UnixSocketServer server("/tmp/system_monitor.sock");
    if (!server.Start()) {
        std::cerr << "Failed to start Unix Socket Server\n";
        return 1;
    }

    // 3. Offload ListenLoop() to a separate worker thread (Server Thread)
    // Reason: accept() inside ListenLoop is a Blocking System Call.
    // If run on the Main Thread, the program would block at accept() and never reach the ShutdownSignal loop.
    std::thread serverThread([&server]() {
        server.ListenLoop();
    });

    // 4. Keep the Service process alive until SIGINT (Ctrl+C) or SIGTERM is received
    while (!ShutdownSignal::IsRequested()) {
        ShutdownSignal::SleepUnlessRequested(std::chrono::seconds(1));
    }

    std::cout << "\nReceived signal: " << ShutdownSignal::LastSignal() << std::endl;

    // 5. Perform Graceful Shutdown
    // - Stop the Server (lower the running_ flag and close serverFd_)
    // - Reclaim the serverThread using join()
    server.Stop();
    if (serverThread.joinable()) {
        serverThread.join();
    }

    std::cout << "Monitor Service stopped." << std::endl;
    return 0;
}

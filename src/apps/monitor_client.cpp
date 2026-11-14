#include "client_data.h"        // Thread-safe data storage from Task 7
#include "signal_handler.h"     // Ctrl+C (SIGINT/SIGTERM) signal handler from Phase 4
#include "unix_socket_client.h" // Self-threaded IPC Client class from Task 7

#include <chrono>   // Provides std::chrono::seconds
#include <iomanip>  // Provides std::fixed and std::setprecision for decimal formatting
#include <iostream> // Provides std::cout and std::cerr
#include <unistd.h> // Provides getpid() and getppid()

int main() {
    // 1. Register system signal handlers to support safe process termination via Ctrl+C
    if (!ShutdownSignal::Install()) {
        std::cerr << "Failed to install signal handlers\n";
        return 1;
    }

    std::cout << "Monitor Client Started (pid=" << getpid() << ", ppid=" << getppid() << ")\n";

    // 2. Initialize the shared ClientData storage and the UnixSocketClient connection manager
    ClientData clientData;
    UnixSocketClient client(clientData, "/tmp/system_monitor.sock");

    // 3. Start the background communication thread (socketThread_ automatically connects and fetches data every 1s)
    if (!client.start()) {
        std::cerr << "[Client] Failed to start the IPC communication thread!\n";
        return 1;
    }

    // 4. Main Thread (UI Thread) primary loop:
    // Simply reads the latest data from ClientData and renders it to the screen every 1 second
    while (!ShutdownSignal::IsRequested() && client.isRunning()) {
        if (clientData.hasData()) {
            // Retrieve a copy of the latest data safely via mutex
            ipc::StatusPayload status = clientData.get();

            // Display metrics on the Console following the exact output format of Task 7
            std::cout << "\033[2J\033[H"; // ANSI escape code to clear the Console screen and move cursor to top-left
            std::cout << "========================================\n";
            std::cout << "       SYSTEM MONITOR (IPC CLIENT)      \n";
            std::cout << "========================================\n";
            std::cout << std::fixed << std::setprecision(1);
            std::cout << "CPU   : " << status.cpu << "%\n";
            std::cout << "MEM   : " << status.memory << "%\n";
            std::cout << "DISK  : " << status.disk << "%\n";
            std::cout << "PROCS : " << status.processCount << "\n";
            std::cout << "========================================\n";
            std::cout << "(Press Ctrl+C to exit)\n" << std::flush;
        } else {
            std::cout << "[Client] Waiting for initial data from Monitor Service...\n";
        }

        // Sleep for 1 second before the next UI render (wakes up immediately if Ctrl+C is pressed)
        ShutdownSignal::SleepUnlessRequested(std::chrono::seconds(1));
    }

    std::cout << "\n[Client] Stopping process...\n";

    // 5. Stop the background communication thread and safely join the thread before exiting main()
    client.stop();

    std::cout << "Monitor Client stopped.\n";
    return 0;
}

#include "monitor_data.h"       // Thread-safe hardware data storage from Phase 3
#include "monitor_worker.h"     // Background thread collecting CPU, RAM, and Disk metrics from Phase 3
#include "signal_handler.h"     // Ctrl+C (SIGINT/SIGTERM) signal handler from Phase 4
#include "unix_socket_server.h" // Self-threaded IPC Server Publisher class from Task 8

#include <chrono>               // Provides std::chrono::seconds
#include <iostream>             // Provides std::cout and std::cerr
#include <unistd.h>             // Provides getpid() and getppid()

int main() {
    // 1. Install signal handlers to support safe Service shutdown via Ctrl+C
    if (!ShutdownSignal::Install()) {
        std::cerr << "Failed to install signal handlers\n";
        return 1;
    }

    std::cout << "Monitor Service Started (pid=" << getpid() << ", ppid=" << getppid() << ")\n";

    // 2. Initialize the shared MonitorData storage
    MonitorData monitorData;

    // 3. Start the Worker thread to read real metrics from /proc every 1 second and write to monitorData
    MonitorWorker worker(monitorData);
    worker.Start();

    // 4. Start the Socket Server thread to proactively push data from monitorData to the Client
    UnixSocketServer server(monitorData, "/tmp/system_monitor.sock");
    if (!server.start()) {
        std::cerr << "[Service Error] Failed to start Unix Socket Server!\n";
        worker.Stop();
        return 1;
    }

    // 5. Main Thread sleeps while waiting for the shutdown signal (zero CPU overhead)
    while (!ShutdownSignal::IsRequested()) {
        ShutdownSignal::SleepUnlessRequested(std::chrono::seconds(1));
    }

    std::cout << "\n[Service] Stopping process...\n";

    // 6. Stop the network communication thread and the data collection thread in reverse order
    server.stop();
    worker.Stop();

    std::cout << "Monitor Service stopped.\n";
    return 0;
}

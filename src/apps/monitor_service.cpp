#include "monitor_data.h"
#include "monitor_worker.h"
#include "signal_handler.h"
#include "unix_socket_server.h"
#include "chrono"
#include "iostream"
#include "unistd.h"

int main () {
    // 1. Install system signal handlers (SIGINT, SIGTERM) to support graceful process shutdown (systemd-ready)
    if(!ShutdownSignal::Install()) {
        std::cerr << "[Service Error] Failed to install signal handlers\n";
        return 1; 
    }
    // Print startup message with Process ID (PID) and Parent Process ID (PPID)
    std::cout << "Monitor Service Started (pid=" << getpid() << ", ppid=" << getppid() << ")\n";

    // 2. Initialize the shared central data store (Thread-safe MonitorData)
    MonitorData monitorData;

    // 3. Start the Worker thread to periodically read hardware metrics from /proc and write to monitorData
    MonitorWorker worker(monitorData);
    worker.Start();

    // 4. Start the Socket Server Thread to listen for and serve Request/Response over Unix Domain Socket
    UnixSocketServer server(monitorData, "/tmp/system_monitor.sock");
    if(!server.start()) {
        std::cerr << "[Service Error] Failed to start UnixSocketServer\n";
        // Stop worker if the server fails to start
        worker.Stop();
        return 1;
    }

    // 5. Main Thread sleeps while waiting for the shutdown signal (zero CPU overhead, checks every second)
    while(!ShutdownSignal::IsRequested()) {
        ShutdownSignal::SleepUnlessRequested(std::chrono::seconds(1));
    }

    // 6. Upon receiving a stop signal (Ctrl+C / SIGTERM), perform a Graceful Shutdown in reverse order
    std::cout << "\n[Service] Stopping process...\n";
    server.stop(); // Stop the server network thread first
    worker.Stop(); // Stop the hardware collection thread afterward

    std::cout << "Monitor Service stopped.\n";
    return 0;
}

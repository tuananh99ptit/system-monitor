#include "signal_handler.h"
#include <chrono>
#include <iostream>
#include <unistd.h>

int main() {
    if (!ShutdownSignal::Install()) {
        std::cerr << "Failed to install signal handlers\n";
        return 1;
    }

    std::cout << "Monitor Service Started (pid=" << getpid() << ", ppid=" << getppid() << ")" << std::endl;

    // TODO Task 8 : start MonitorWorker (already implemented in Phase 3)
    // TODO Task 4/8: start UnixSocketServer

    // Keep the service alive until SIGINT / SIGTERM is received
    while (!ShutdownSignal::IsRequested()) {
        ShutdownSignal::SleepUnlessRequested(std::chrono::seconds(1));
    }

    std::cout << "\nReceived signal: " << ShutdownSignal::LastSignal() << std::endl;

    // TODO: server.Stop(), worker.Stop()....

    std::cout << "Monitor Service stopped." << std::endl;
    return 0;
}

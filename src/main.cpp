#include "monitor_worker.h"
#include "console_renderer.h"
#include "signal_handler.h"

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    // 1. Register signal handlers immediately upon application startup
    if(!ShutdownSignal::Install()) {
        std::cerr << "Error: Failed to install signal handlers\n";
        return 1;
    }

    // 2. Initialize the data model and components
    MonitorData data;
    MonitorWorker worker(data);
    ConsoleRenderer renderer(data);

    // 3. Start the background worker thread for data collection
    worker.Start();

    // Wait for 100ms for the worker to load the first batch of data, avoiding 0% on the initial frame
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 4. Main Thread Loop (UI): Runs until IsRequested() == true
    // Main thread = UI thread. The loop terminates upon receiving SIGINT/SIGTERM.
    while(!ShutdownSignal::IsRequested()) {
        renderer.Render();

        // Sleep for 1 second flexibly, responding instantly if Ctrl+C is pressed
        ShutdownSignal::SleepUnlessRequested(std::chrono::seconds(1));
    }
    
    // 5. GRACEFUL SHUTDOWN STEP
    std::cout << "\nReceived signal: " << ShutdownSignal::LastSignal() << std::endl;

    // Stop the background worker thread and safely join() before object destruction
    worker.Stop();

    std::cout << "System Monitor stopped cleanly." << std::endl;
    return 0;
}

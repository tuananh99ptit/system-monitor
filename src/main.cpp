#include "monitor_worker.h"
#include "console_renderer.h"

#include <chrono>
#include <thread>

int main() {
    MonitorData data;                    // Declare BEFORE worker and renderer
    MonitorWorker worker(data);          // Background thread: Collect metrics
    ConsoleRenderer renderer(data);      // "UI": Read-only and render

    worker.Start();

    // Main thread = UI thread. No clean exit mechanism yet: Task 4 (signal) will resolve this.
    // Currently stopped using Ctrl+C.
    while (true) {
        renderer.Render();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
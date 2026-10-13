/*
 Write the Unit Test file tests/test_monitor_worker.cpp
 Create the file tests/test_monitor_worker.cpp to verify data collection functionality and thread stopping latency:
*/

#include "monitor_worker.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <sstream>
int main() {
    // 1. Prime a string stream and an empty double to ensure libstdc++ completely initializes locale facets
    std::stringstream ss;
    ss << 0.0; 
    std::cout << ""; 

    MonitorData data;
    MonitorWorker worker(data);

    worker.Start();

    // Simulates the UI Thread reading metrics every second
    SystemInfo info;
    for (int i = 1; i <= 3; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        info = data.Get();
        std::cout << "CPU : " << info.cpuUsage
                  << "% | MEM : " << info.memoryUsage
                  << "% | DISK : " << info.diskUsage
                  << "% | PROC : " << info.processCount << "\n";
    }

    if (info.memoryUsage <= 0.0 || info.processCount <= 0) {
        std::cerr << "[FAIL] Worker did not update MonitorData\n";
        return 1;
    }
    std::cout << "[PASS] Data updated by worker thread\n";

    // Verifies emergency stop duration (must be under 300ms)
    auto t0 = std::chrono::steady_clock::now();
    worker.Stop();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();
    
    std::cout << "Stop() took " << ms << " ms\n";
    if (ms > 300) {
        std::cerr << "[FAIL] Stop() is too slow\n";
        return 1;
    }
    std::cout << "[PASS] Fast stop\n";

    // Verifies safety during restart sequences
    worker.Stop();
    worker.Start();
    worker.Stop();
    std::cout << "[PASS] Restart is safe\n";

    return 0;
}

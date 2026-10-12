/*
Concept: The Writer always writes the same value 'i' across all 4 fields. 
If the Reader ever encounters a dataset where the 4 fields are not identical, it indicates a torn read. 
With a mutex in place, this should never happen.
*/

#include "monitor_data.h"
#include <atomic>
#include <iostream>
#include <thread>

int main() {
    MonitorData data;
    
    // Test 1: Before the first Update(), Get() must return zeros (not garbage data)
    SystemInfo initial = data.Get();
    if (initial.cpuUsage != 0.0 || initial.processCount != 0) {
        std::cerr << "[FAIL] Default values are not zero\n";
        return 1;    
    }
    std::cout << "[PASS] Default value\n";

    // Test 2: 1 Writer and 1 Reader running concurrently
    // 1 million iterations to increase the probability of thread collision
    constexpr int kIterations  = 1000000;   
    std::atomic<bool> done{false};          // Flag for the Writer to signal the Reader: "Writing finished, stop now"
    long reads = 0;                         // Counter for total read operations, used for stats only
    long inconsistent = 0;                  // Counter for detected torn reads. Expected result: 0

    // Write thread
    std::thread writer([&] {
        for (int i = 1; i <= kIterations; ++i) {
            SystemInfo info;
            info.cpuUsage     = i;          // All 4 fields are always equal
            info.memoryUsage  = i;
            info.diskUsage    = i;
            info.processCount = i;
            data.Update(info);
        }
        done = true;
    });

    // Read Thread
    std::thread reader([&] {
        while(!done) {
            SystemInfo s = data.Get();
            ++reads;
            if (s.cpuUsage != s.memoryUsage || s.cpuUsage != s.diskUsage || s.cpuUsage != s.processCount) {
                ++inconsistent;
            }
        }
    });

    writer.join();
    reader.join();

    std::cout << "Reads: " << reads << ", inconsistent: " << inconsistent << "\n";
    if (inconsistent != 0) {
        std::cerr << "[FAIL] Torn reads detected!\n";
        return 1;
    }
    std::cout << "[PASS] No torn reads\n";
    return 0;
}

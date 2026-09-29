#include "cpu/cpu_monitor.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
int main() {
    CpuMonitor monitor;

    // Prepare the initial timestamp before entering the loop
    monitor.GetCpuUsage(); 

    while (true) {
        // Wait 500ms to let the system accumulate the time difference

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        //Calculate the actual CPU % based on the difference from the previous reading
        float cpu = monitor.GetCpuUsage();

        // Clear the screen and move the cursor to the top-left corner
        std::cout << "\033[2J\033[1;1H";

        // 4. In kết quả 
        std::cout << "=== LINUX SYSTEM MONITOR ===" << std::endl;
        std::cout << "+----------------------+\n";
        std::cout << "| CPU Usage : " << std::setw(3) << static_cast<int>(cpu) << "%       |\n";
        std::cout << "+----------------------+\n";
    }

    return 0;
}

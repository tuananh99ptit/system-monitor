<<<<<<< HEAD
#include "memory/memory_monitor.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
int main(){
    
    MemoryMonitor monitor;
    while(true) {
        // Fetch current system memory information
        auto info = monitor.GetMemoryInfo();
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "+-----------------------------+\n";
        std::cout << "| Total Memory : " << KbToGb(info.total) << " GB\n";
        std::cout << "| Used Memory  : " << KbToGb(info.used) << " GB\n";
        std::cout << "| Free Memory  : " << KbToGb(info.free) << " GB\n";
        std::cout << "+-----------------------------+\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
=======
#include <iostream>

int main() {
    std::cout << "=== Linux System Monitor Initialized ===" << std::endl;
>>>>>>> origin/main
    return 0;
}

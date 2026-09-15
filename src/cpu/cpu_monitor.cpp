#include "cpu_monitor.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
// constructor: initializes all initial timestamps to zero
CpuMonitor::CpuMonitor() {
    prevIdleTime = 0;
    prevTotalTime = 0;
}
float CpuMonitor::GetCpuUsage() {
    // open the /proc/stat system file
    std::ifstream file("/proc/stat");
    if(!file.is_open()){
        return 0.0f;
    }
    // read the first line, which contains the overall CPU data
    std::string line;
    std::getline(file,line);
    file.close();
    // split the string/line into individual integers
    std::istringstream ss(line);
    std::string CpuLabel;
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    // read the CpuLabel along with its 8 corresponding CPU tick values
    ss >> CpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal ;
    // Calculate IdleTime and TotalTime at the current point in time (T2)
    unsigned long long idleTime = idle + iowait;
    unsigned long long totalTime = user + nice + system + idle + iowait + irq + softirq + steal;
    // calculate the delta difference between this reading and the previous one
    unsigned long long totalDiff = totalTime - prevTotalTime;
    unsigned long long idleDiff = idleTime - prevIdleTime;
    // Update the old timestamp for use in the next call to GetCpuUsage()
    prevTotalTime = totalTime;
    prevIdleTime = idleTime;
    // Avoid division-by-zero error if the function is called twice in quick succession
    if (totalDiff == 0) {
        return 0.0f;
    }
    // Apply the formula: % CPU = (1 - ΔIdle / ΔTotal) * 100
    double usageRatio = 1.0 - static_cast<double>(idleDiff) / totalDiff;
    return static_cast<float>(usageRatio * 100.0);
}
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
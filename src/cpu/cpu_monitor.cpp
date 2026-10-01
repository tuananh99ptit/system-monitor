#include "cpu_monitor.h"
#include <fstream>
#include <sstream>
#include <string>
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
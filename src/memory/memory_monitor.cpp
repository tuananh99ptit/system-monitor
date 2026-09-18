#include "memory_monitor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
long MemoryMonitor::GetValueByLabel(const std::string &content, const std::string &label){
    // Find position of target label (e.g. "MemTotal:") in file content
    size_t pos = content.find(label);
    if(pos == std::string::npos){
        return 0; // not find the label -> return 0
    }
    // Jump to the target value position
    pos = pos + label.length();
    //Parse the first integer following the label using stringstream (skip "kB")
    std::basic_istringstream iss(content.substr(pos));
    long value = 0;
    iss >> value;
    return value; // KB
}

MemoryInfo MemoryMonitor::GetMemoryInfo(){
    MemoryInfo info;
    info.total = 0;
    info.used = 0;
    info.free = 0;
    // open systen file /proc/meminfo and check error
    std::ifstream file("/proc/meminfo");
    if(!file.is_open()){
        std::cerr << "Failed to open /proc/meminfo" << std::endl;
        return info;
    }
    // Read the entire file content into a string for label-based parsing
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    // Extract required values (in KB)
    long memTotal = GetValueByLabel(content, "MemTotal:");
    long memAvailable = GetValueByLabel(content, "MemAvailable:");
    // Fallback for older kernels without MemAvailable support:
    // Estimate as MemFree + Buffers + Cached + SReclaimable
    if (memAvailable == 0){
        long memFree = GetValueByLabel(content, "MemFree:");
        long buffers = GetValueByLabel(content, "Buffers:");
        long cached = GetValueByLabel(content, "Cached:");
        // SReclaimable: Slab memory (dentry/inode cache) that can be reclaimed
        // immediately when needed, so it counts toward "available" memory
        long sReclaimable = GetValueByLabel(content, "SReclaimable:");
        memAvailable = memFree + buffers + cached + sReclaimable;
    }
    // Calculate usage metrics: Used = Total - Available
    info.total = memTotal;
    info.free = memAvailable;
    info.used = memTotal - memAvailable;
    return info;
}
// Utility function: convert KB to GB for display
double KbToGb(long kb){
    return static_cast<double>(kb) / (1024.0 * 1024.0);
}

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
    return 0;
}
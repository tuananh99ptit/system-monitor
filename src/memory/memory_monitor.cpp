#include "memory_monitor.h"
#include "file_reader.h"
#include "logger.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <iostream>

long MemoryMonitor::GetValueByLabel(const std::vector<std::string>& lines, const std::string& label){
    // Find position of target label (e.g. "MemTotal:") in file content
    for(const auto& line:lines) {
        size_t pos = line.find(label);
            if(pos!= std::string::npos) {
                 std::istringstream iss(line.substr(pos + label.length()));
                 long value = 0;
                 iss >> value;
                 return value;
            }
    }
    return 0;
}

MemoryInfo MemoryMonitor::GetMemoryInfo(){
    MemoryInfo info;
    info.total = 0;
    info.used = 0;
    info.free = 0;
    // open systen file /proc/meminfo and check error
    // 1. Use FileReader to read lines from /proc/meminfo
    std::vector<std::string> lines = FileReader::ReadLines("/proc/meminfo");
    
    if (lines.empty()) {
        // FileReader::ReadLines already logs Logger::Error if the file cannot be opened.
        // Log an additional business-level warning for the Memory module:
        Logger::Warn("Failed to calculate memory info because /proc/meminfo content is empty");
        return info;
    }

    // 2. Extract values by labels (in KB)

    long memTotal = GetValueByLabel(lines, "MemTotal:");
    long memAvailable = GetValueByLabel(lines, "MemAvailable:");
    // Fallback for older kernels without MemAvailable support:
    // Estimate as MemFree + Buffers + Cached + SReclaimable
    if (memAvailable == 0){
        long memFree = GetValueByLabel(lines, "MemFree:");
        long buffers = GetValueByLabel(lines, "Buffers:");
        long cached = GetValueByLabel(lines, "Cached:");
        // SReclaimable: Slab memory (dentry/inode cache) that can be reclaimed
        // immediately when needed, so it counts toward "available" memory
        long sReclaimable = GetValueByLabel(lines, "SReclaimable:");
        memAvailable = memFree + buffers + cached + sReclaimable;
    }


    // 3. Calculate memory metrics: Used = Total - Available
    info.total = memTotal;
    info.free = memAvailable;
    info.used = memTotal - memAvailable;

    return info;
}
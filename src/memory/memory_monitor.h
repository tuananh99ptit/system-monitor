#ifndef MEMORY_MONITOR_H
#define MEMORY_MONITOR_H

#include <string>
#include <vector>

// Structure to store memory information in KB, read directly from /proc/meminfo
struct MemoryInfo{
    long total; // total RAM capacity (Memtotal)
    long used; // Used memory capacity (Total - Free - Buffers - Cached - SReclaimable)
    long free; // Available free memory capacity (MemAvailable)
};
class MemoryMonitor {
    public : 
    // Read /proc/meminfo and return current memory information
    MemoryInfo GetMemoryInfo();

    private:
    // Find the KB value of a specific label in /proc/meminfo
    // Example: GetValueByLabel(content, "MemTotal")
    long GetValueByLabel(const std::vector<std::string>& lines, const std::string& lablel);
};
#endif // MEMORY_MONITOR_H
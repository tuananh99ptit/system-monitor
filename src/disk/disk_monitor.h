#ifndef DISK_MONITOR_H
#define DISK_MONITOR_H
#include <string>
// struct to stote disk capacity information (unit : bytes)
struct DiskInfo{
    unsigned long long total; // Total disk capacity 
    unsigned long long used; // Used disk capacity
    unsigned long long free; // Free / Available disk capacity
};
class DiskMonitor{
    public:
    // Returns the percentage of disk space currently in use (0.0 - 100.0)
    float GetDiskUsage();

    // Returns detailed Total - Used - Free information in bytes for the monitored path
    DiskInfo GetDiskInfo();

    private:
    // Monitored mount point path, default is "/" - root partition
    std::string path_ = "/";
};
#endif // DISK_MONITOR_H
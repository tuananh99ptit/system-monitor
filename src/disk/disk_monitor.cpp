#include "disk_monitor.h"
#include <sys/statvfs.h>
#include <iostream>
#include <iomanip>
#include <thread>
DiskInfo DiskMonitor::GetDiskInfo(){
    
    DiskInfo info;
    info.total = 0;
    info.used = 0;
    info.free = 0;
    
    struct statvfs stat;
    // statvfs() reads filesystem infomation at path_ ex: /
    if(statvfs(path_.c_str(), &stat) != 0){
        std::cerr << "Failed to read disk infomation at: " << path_ << std::endl;
        return info;
    } 

    // f_frsize : fundament filesystem block size (fragment size) in bytes 
    // f_blocks : total numbers of blocks in the filesystem
    unsigned long long blockSize = stat.f_frsize;
    
    // total capacity = total block count * size per block
    info.total = static_cast<unsigned long long>(stat.f_blocks) * blockSize;

    // f_bfree : total number of free blocks including root-reserved blocks
    // use f_bfree to caculate used-space  to match the df command logic
    unsigned long long freeAll = static_cast<unsigned long long>(stat.f_bfree) * blockSize;
    
    // f_bavail: number of free blocks available to unprivileged (non-root) users
    // This represents the actual "Free" space meaningful to the end user (similar
    // to the "Avail" column in "df"), as the root-reserved portion cannot be used by regular users
    info.free = static_cast<unsigned long long>(stat.f_bavail) * blockSize;

    // Used capacity = Total - TOTAL free blocks (including root-reserved space)
    // Do NOT use Total - f_bavail, as that would wrongly count root-reserved
    // space as "Used", causing an artificially high usage percentage
    info.used = info.total - freeAll;

    return info;
}


float DiskMonitor::GetDiskUsage() {
    DiskInfo info = GetDiskInfo();

    // The denominator uses (Used + Free) instead of Total, matching how the "df"
    // command calculates Use%. Reason: Total includes blocks reserved exclusively
    // for root that regular users cannot access. Dividing by Total would yield a %
    // significantly lower than what "df -h" displays.
    unsigned long long denominator = info.used + info.free;

    // Prevent division by zero if both Used and Free evaluate to 0 (error case)
    if (denominator == 0)
    {
        return 0.0f;
    }

    return (static_cast<float>(info.used) / static_cast<float>(denominator)) * 100.0f;
}

double BytesToGb(unsigned long long bytes){
    return static_cast<double>(bytes) / (1024 * 1024 * 1024);
}

int main()
{
    DiskMonitor monitor;
    while(true){
    DiskInfo info = monitor.GetDiskInfo();
    auto usage = monitor.GetDiskUsage();
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "+-----------------------------+\n";
    std::cout << "| Total : " << BytesToGb(info.total) << " GB\n";
    std::cout << "| Used  : " << BytesToGb(info.used) << " GB\n";
    std::cout << "| Free  : " << BytesToGb(info.free) << " GB\n";
    std::cout << "+-----------------------------+\n";
    std::cout << "Disk Usage: " << usage << " %\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
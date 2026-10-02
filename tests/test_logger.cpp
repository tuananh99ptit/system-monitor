#include "logger.h"
#include "cpu_monitor.h"
#include "memory_monitor.h"
#include "disk_monitor.h"
#include "config.h"
#include <thread>
#include <chrono>
#include <sstream>

// 1. Hàm kiểm tra và ghi log CPU
void CheckAndLogCPU(CpuMonitor& cpuMon) {
    double cpuPercent = cpuMon.GetCpuUsage();
    std::ostringstream ss;
    ss << "CPU=" << static_cast<int>(cpuPercent) << "%";

    if (cpuPercent >= 85.0) {
        Logger::Warn(ss.str());
    } else {
        Logger::Info(ss.str());
    }
}

// 2. Hàm kiểm tra và ghi log Memory
void CheckAndLogMemory(MemoryMonitor& memMon) {
    auto memInfo = memMon.GetMemoryInfo();
    double memPercent = 0.0;
    if (memInfo.total > 0) {
        memPercent = (memInfo.used / memInfo.total) * 100.0;
    }

    std::ostringstream ss;
    ss << "MEM=" << static_cast<int>(memPercent) << "%";

    if (memPercent >= 80.0) {
        Logger::Warn(ss.str());
    } else {
        Logger::Info(ss.str());
    }
}

// 3. Hàm kiểm tra và ghi log Disk
void CheckAndLogDisk(DiskMonitor& diskMon) {
    auto diskInfo = diskMon.GetDiskInfo();
    double diskPercent = 0.0;
    if (diskInfo.total > 0) {
        diskPercent = (diskInfo.used / diskInfo.total) * 100.0;
    }

    std::ostringstream ss;
    ss << "DISK=" << static_cast<int>(diskPercent) << "%";

    if (diskPercent >= 90.0) { // Ngưỡng Disk >= 90% sẽ ghi WARN
        Logger::Warn(ss.str());
    } else {
        Logger::Info(ss.str());
    }
}

int main() {
    CpuMonitor cpuMon;
    MemoryMonitor memMon;
    DiskMonitor diskMon;

    while(true) {
        CheckAndLogCPU(cpuMon);
        CheckAndLogMemory(memMon);
        CheckAndLogDisk(diskMon);

        // Nghỉ 1 giây (1000ms) trước khi sang chu kỳ quét tiếp theo
        std::this_thread::sleep_for(std::chrono::milliseconds(Config::DEFAULT_REFRESH_INTERVAL_MS));
    }
    return 0;
}
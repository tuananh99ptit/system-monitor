#include "cpu/cpu_monitor.h"
#include "memory/memory_monitor.h"
#include "disk/disk_monitor.h"
#include "process/process_monitor.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

int main() {
    // Initialize all 4 monitors - each class handles exactly 1 task (Single Responsibility Principle)
    CpuMonitor cpuMonitor;
    MemoryMonitor memMonitor;
    DiskMonitor diskMonitor;
    ProcessMonitor procMonitor;

    // The CPU Monitor requires an initial "priming" read before the loop to establish the T1 timestamp,
    // preventing a division-by-zero error on the first percentage calculation.
    cpuMonitor.GetCpuUsage();

    while (true) {
        // Wait for 1 second between each complete dashboard refresh
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // ===== Fetch data from all 4 modules BEFORE rendering to the screen =====
        // (Fetch all data first, print later - this prevents screen flickering mid-render 
        // while calculating heavy metrics like scanning the /proc directory)
        float cpuUsage = cpuMonitor.GetCpuUsage();
        MemoryInfo memInfo = memMonitor.GetMemoryInfo();
        DiskInfo diskInfo = diskMonitor.GetDiskInfo();
        float diskUsage = diskMonitor.GetDiskUsage();

        std::vector<ProcessInfo> processes = procMonitor.GetProcesses();

        // Clear the screen and reset the cursor to the top-left corner (0,0)
        std::cout << "\033[2J\033[1;1H";

        std::cout << std::fixed << std::setprecision(2);

        std::cout << "================================================================\n";
        std::cout << "         LINUX SYSTEM MONITOR\n";
        std::cout << "================================================================\n\n";

        // ----- 1. CPU -----
        std::cout << "[ CPU ]\n";
        std::cout << "+----------------------+\n";
        std::cout << "| Usage : " << std::setw(5) << cpuUsage << " %   |\n";
        std::cout << "+----------------------+\n\n";

        // ----- 2. MEMORY -----
        std::cout << "[ MEMORY ]\n";
        std::cout << "+-----------------------------+\n";
        std::cout << "| Total : " << std::setw(7) << KbToGb(memInfo.total) << " GB      |\n";
        std::cout << "| Used  : " << std::setw(7) << KbToGb(memInfo.used) << " GB      |\n";
        std::cout << "| Free  : " << std::setw(7) << KbToGb(memInfo.free) << " GB      |\n";
        std::cout << "+-----------------------------+\n\n";

        // ----- 3. DISK -----
        std::cout << "[ DISK ]\n";
        std::cout << "+-----------------------------+\n";
        std::cout << "| Total : " << std::setw(7) << BytesToGb(diskInfo.total) << " GB      |\n";
        std::cout << "| Used  : " << std::setw(7) << BytesToGb(diskInfo.used) << " GB      |\n";
        std::cout << "| Free  : " << std::setw(7) << BytesToGb(diskInfo.free) << " GB      |\n";
        std::cout << "| Usage : " << std::setw(7) << diskUsage << " %       |\n";
        std::cout << "+-----------------------------+\n\n";

        // ----- 4. PROCESS LIST -----
        std::cout << "[ PROCESSES ]\n";

        // Automatically calculate the "Name" column width based on the longest active process name,
        // ensuring the table stays perfectly aligned regardless of varying name lengths.
        size_t maxNameLength = 4; // Default minimum length matches the word "Name"
        for (const auto &p : processes) {
            if (p.name.length() > maxNameLength) {
                maxNameLength = p.name.length();
            }
        }

        const size_t pidWidth = 7;
        const size_t nameWidth = maxNameLength;
        const size_t stateWidth = 10;

        std::string separator = "+" + std::string(pidWidth + 2, '-') +
                                 "+" + std::string(nameWidth + 2, '-') +
                                 "+" + std::string(stateWidth + 2, '-') + "+";

        std::cout << separator << "\n";
        std::cout << "| " << std::left << std::setw(pidWidth) << "PID"
                   << " | " << std::left << std::setw(nameWidth) << "Name"
                   << " | " << std::left << std::setw(stateWidth) << "State"
                   << " |\n";
        std::cout << separator << "\n";

        for (const auto &p : processes) {
            std::cout << "| " << std::left << std::setw(pidWidth) << p.pid
                       << " | " << std::left << std::setw(nameWidth) << p.name
                       << " | " << std::left << std::setw(stateWidth) << p.state
                       << " |\n";
        }

        std::cout << separator << "\n";
        std::cout << "Total processes: " << processes.size() << "\n";

        // Force the output buffer to flush immediately. 
        // This is critical for maintaining a seamless, real-time updated dashboard interface.
        std::cout.flush();
    }
    return 0;
}

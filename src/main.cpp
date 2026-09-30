#include "process/process_monitor.h"
#include <iostream>
#include <iomanip>

int main()
{
    ProcessMonitor monitor;

    // Lấy danh sách toàn bộ tiến trình hiện có trên hệ thống
    std::vector<ProcessInfo> processes = monitor.GetProcesses();

    std::cout << "+------+---------------------+-------------+\n";
    std::cout << "| PID  | Name                | State       |\n";
    std::cout << "+------+---------------------+-------------+\n";

    for (const auto &p : processes)
    {
        std::cout << "| " << std::left << std::setw(4) << p.pid
                   << " | " << std::setw(19) << p.name
                   << " | " << std::setw(11) << p.state
                   << " |\n";
    }

    std::cout << "+------+---------------------+-------------+\n";
    std::cout << "Tong so tien trinh: " << processes.size() << "\n";

    return 0;
}
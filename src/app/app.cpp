#include "app.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

void App::Display() {
    DiskInfo info = monitor.GetDiskInfo();
    auto usage = monitor.GetDiskUsage();
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "+-----------------------------+\n";
    std::cout << "| Total : " << BytesToGb(info.total) << " GB\n";
    std::cout << "| Used  : " << BytesToGb(info.used) << " GB\n";
    std::cout << "| Free  : " << BytesToGb(info.free) << " GB\n";
    std::cout << "+-----------------------------+\n";
    std::cout << "Disk Usage: " << usage << " %\n";
}

void App::Run() {
    while(true) {
        Display();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
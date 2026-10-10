#ifndef APP_H
#define APP_H

#include "cpu/cpu_monitor.h"
#include "memory/memory_monitor.h"
#include "disk/disk_monitor.h"
#include "process/process_monitor.h"

class App {
public:
    App() = default;
    ~App() = default;

    // // Responsible for running the main application loop
    void Run();
private:

    // Initialize all 4 monitors - each class handles exactly 1 task (Single Responsibility Principle)
    CpuMonitor cpuMonitor;
    MemoryMonitor memMonitor;
    DiskMonitor diskMonitor;
    ProcessMonitor procMonitor;

    // Dedicated function responsible for displaying the console interface
    void Display();
};

#endif // APP_H

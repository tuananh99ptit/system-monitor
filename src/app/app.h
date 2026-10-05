#ifndef APP_H
#define APP_H

#include "cpu_monitor.h"

class App {
public:
    App() = default;
    // Responsible for running the main application loop
    void Run();

private:
    CpuMonitor cpuMonitor_;
    // Dedicated function responsible for displaying the console interface
    void Display(float cpuUsage);
};

#endif //APP_H
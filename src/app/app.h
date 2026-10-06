#ifndef APP_H
#define APP_H

#include "disk_monitor.h"

class App {
public:
    App() = default;
    ~App() = default;

    // Responsible for running the main application loop
    void Run();
private:
    DiskMonitor monitor;
    //  Dedicated function responsible for displaying the console interface
    void Display();
};

#endif //APP_H
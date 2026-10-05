#ifndef APP_H
#define APP_H

#include "memory/memory_monitor.h"
class App {
public:
    App() = default;
    ~App() = default;

    // Responsible for running the main application loop
    void Run();
private:
    MemoryMonitor monitor_;
    // Dedicated function responsible for displaying the console interface
    void Display();
};

#endif // APP_H
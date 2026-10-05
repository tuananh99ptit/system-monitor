#include "app.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

// Separate the display functionality into a dedicated function
void App::Display(float cpuUsage) {
    // Clear screen and move cursor to top-left
    std::cout << "\033[2J\033[1;1H";

    // Print the Monitor interface
    std::cout << "=== LINUX SYSTEM MONITOR ===" << std::endl;
    std::cout << "+----------------------+\n";
    std::cout << "| CPU Usage : " << std::setw(3) << static_cast<int>(cpuUsage) << "%        |\n";
    std::cout << "+----------------------+\n";
}

// Move the main loop into App::Run()
void App::Run() {
    // Initialize the initial sampling point
    cpuMonitor_.GetCpuUsage();

    while (true) {
        // Pause for 500ms to measure the CPU tick difference
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Get CPU percentage
        float cpu = cpuMonitor_.GetCpuUsage();

        // Call the separate display function
        Display(cpu);
    }
}

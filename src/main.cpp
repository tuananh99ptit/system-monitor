#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include "process/process_monitor.h"

int main() {
    ProcessMonitor monitor;
    while(true) {

    // Clear the terminal screen and move the cursor to the top-left corner (0,0)
    std::cout << "\033[2J\033[1;1H";    
    
    // Retrieve the active process list
    std::vector<ProcessInfo> processes = monitor.GetProcesses();

    // 1. Determine the maximum width for the Name column based on actual data
    size_t maxNameLength = 4; // Default minimum length matches the word "Name"
    for (const auto& p : processes) {
        if (p.name.length() > maxNameLength) {
            maxNameLength = p.name.length();
        }
    }

    // Define column widths (The Name column scales dynamically based on maxNameLength)
    const size_t pidWidth = 7;
    const size_t nameWidth = maxNameLength; 
    const size_t stateWidth = 10;

    // 2. Automatically generate a horizontal separator line that matches the table width
    std::string separator = "+" + std::string(pidWidth + 2, '-') +
                            "+" + std::string(nameWidth + 2, '-') +
                            "+" + std::string(stateWidth + 2, '-') + "+";

    // 3. Print Header
    std::cout << separator << "\n";
    std::cout << "| " << std::left << std::setw(pidWidth) << "PID"
              << " | " << std::left << std::setw(nameWidth) << "Name"
              << " | " << std::left << std::setw(stateWidth) << "State"
              << " |\n";
    std::cout << separator << "\n";

    // 4. Print the process list (Displays full names without truncation)
    for (const auto& p : processes) {
        std::cout << "| " << std::left << std::setw(pidWidth) << p.pid
                  << " | " << std::left << std::setw(nameWidth) << p.name
                  << " | " << std::left << std::setw(stateWidth) << p.state
                  << " |\n";
    }

    // 5. Print Footer
    std::cout << separator << "\n";
    std::cout << "Total processes: " << processes.size() << "\n";


    // 6. Sleep for 1 second before the next refresh
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}

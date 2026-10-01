#include "process_monitor.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cctype> // std::isdigit

std::string ProcessMonitor::ConVertStateChar(char stateChar) {
    switch (stateChar) {
        case 'R': return "RUN";
        case 'S': return "SLEEP";
        case 'D': return "DISK_SLEEP";   // Uninterruptible sleep, usually waiting for I/O
        case 'I': return "IDLE";         // Idle kernel thread, no work to do
        case 'Z': return "ZOMBIE";
        case 'T': return "STOP";
        case 't': return "TRACE_STOP";
        case 'X':
        case 'x': return "DEAD";
        default:  return "UNKNOWN";
    }
}

bool ProcessMonitor::ReadProcessStat(int pid, ProcessInfo &outInfo) {
    std::string statPath = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream file(statPath);

    if (!file.is_open()) {
        // The process might have terminated right when we opened the file (race condition)
        return false;
    }

    // Read the single entire line inside /proc/[pid]/stat
    std::string line;
    std::getline(file, line);

    // Find the FIRST '(' and the LAST ')' to accurately extract the process name,
    // as the name itself may contain spaces or parentheses inside.
    size_t firstParen = line.find('(');
    size_t lastParen = line.rfind(')');

    if (firstParen == std::string::npos || lastParen == std::string::npos || lastParen < firstParen) {
        return false; // Malformed line format, skip safely
    }

    // The process name is located between these two parentheses
    std::string name = line.substr(firstParen + 1, lastParen - firstParen - 1);

    // The state character is located 2 characters after the last ')' (separated by a space)
    // Example: ...) S 1000 ... -> lastParen points to ')', +2 jumps over ')' and the space
    char stateChar = line[lastParen + 2];

    outInfo.pid = pid;
    outInfo.name = name;
    outInfo.state = ConVertStateChar(stateChar);

    return true;
}

std::vector<ProcessInfo> ProcessMonitor::GetProcesses() {
    std::vector<ProcessInfo> processes;

    // Iterate through all entries (files/directories) inside /proc using std::filesystem
    // (C++17 standard library, replacing the legacy C-style opendir()/readdir() approach)
    for (const auto &entry : std::filesystem::directory_iterator("/proc")) {
        if (!entry.is_directory()) {
            continue; // Skip, only interested in directories (each PID is a directory)
        }

        std::string folderName = entry.path().filename().string();

        // /proc also contains many non-PID directories, such as 'self', 'net', 'sys', etc.
        // Only directories whose names consist ENTIRELY OF DIGITS are valid PIDs.
        bool isAllDigits = !folderName.empty() && std::all_of(folderName.begin(), folderName.end(), [](unsigned char c) {
            return std::isdigit(c);
        });

        if (!isAllDigits) {
            continue;
        }

        int pid = std::stoi(folderName);
        ProcessInfo info;

        if (ReadProcessStat(pid, info)) {
            processes.push_back(info);
        }
        // If ReadProcessStat returns false (process just terminated), skip safely.
        // Do not add to the list and do not log an error, as this is expected behavior.
    }

    return processes;
}

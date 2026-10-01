#ifndef PROCESS_MONITOR_H
#define PROCESS_MONITOR_H

#include <string>
#include <vector>

// Information of a single process
struct ProcessInfo {
    int pid;            // Process ID
    std::string name;   // Process name (comm extracted from /proc/[pid]/stat)
    std::string state;  // Human - readable state string (RUN, SLEEP, ZOMBIE....)
};

class ProcessMonitor {
public:
    // Scan the entire /proc directory and return a list of all active processes
    std::vector<ProcessInfo> GetProcesses();
private:
    // Read /proc/[pid]/stat and parse state information for a specific PID.
    // Returns true if successful, false if the process has vanished 
    // (handles race condition where the process might terminate while reading)
    bool ReadProcessStat(int pid, ProcessInfo &outInfo);

    // Convert raw state character (R, S, D, Z, T, etc.) into a readable string
    std::string ConVertStateChar(char stateChar);
};
#endif // PROCESS_MONITOR_H
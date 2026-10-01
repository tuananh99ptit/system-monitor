#ifndef CPU_MONITOR_H
#define CPU_MONITOR_H
class CpuMonitor {
    public:
    // Constructor initializes the initial timestamp to 0
    CpuMonitor();
    // calculate and return the % CPU currently in use
    float GetCpuUsage();
    private:
    // Store the Idle Time and Total Time from the previous reading
    unsigned long long prevIdleTime;
    unsigned long long prevTotalTime;
};
#endif // CPU_MONITOR_H
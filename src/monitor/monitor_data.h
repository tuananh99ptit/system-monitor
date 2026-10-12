#ifndef MONITOR_DATA_H
#define MONITOR_DATA_H

#include <mutex>

// Snapshot of system metrics at a specific point in time
struct SystemInfo {
    double cpuUsage = 0.0;    // %
    double memoryUsage = 0.0; // %
    double diskUsage = 0.0;   // %
    int processCount = 0;
};

// Shared data store between the Worker Thread (write) and the Main Thread (read).
// All accesses go through a mutex, making it thread-safe when called from multiple threads.
class MonitorData {
public:
    MonitorData() = default;
    // Mutex is non-copyable => MonitorData cannot be copied either.
    // Must be passed by reference to ensure all threads share the SAME instance.
    MonitorData(const MonitorData&) = delete;
    MonitorData& operator=(const MonitorData&) = delete;

    // Writes new metrics (called by Worker Thread)
    void Update(const SystemInfo& info);
    // Reads a copy of the current metrics (called by Main Thread)
    SystemInfo Get() const;

private:
    // mutable: allows locking the mutex even inside const functions (like Get)
    mutable std::mutex mutex_;
    SystemInfo info_;
};

#endif //MONITOR_DATA_H
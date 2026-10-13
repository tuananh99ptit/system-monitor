#ifndef MONITOR_WORKER_H
#define MONITOR_WORKER_H

#include "monitor_data.h"
#include "cpu_monitor.h"
#include "memory_monitor.h"
#include "disk_monitor.h"
#include "process_monitor.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

// Background Worker Thread: collects CPU/Memory/Disk/Process metrics every second
// and updates the MonitorData instance.
class MonitorWorker {
public:
    // 'data' must OUTLIVE the worker instance
    explicit MonitorWorker(MonitorData& data);
    ~MonitorWorker();                    // Automatically calls Stop() -> ensures thread joining (RAII)

    // std::thread is non-copyable => disable copy constructor and assignment operator
    MonitorWorker(const MonitorWorker&) = delete;
    MonitorWorker& operator=(const MonitorWorker&) = delete;

    void Start();
    void Stop();

private:
    void Run();                          // Loop function executed inside the worker thread
    SystemInfo Collect();                // Reads all 4 monitors and packages them into a SystemInfo struct

    MonitorData& data_;

    // Private monitors confined to the Worker Thread (Thread Confinement)
    CpuMonitor     cpu_;
    MemoryMonitor  memory_;
    DiskMonitor    disk_;
    ProcessMonitor process_;

    // Flag controlling the collection loop
    std::atomic<bool>       running_{false}; 

    // mutex_ & cv_: Used together in cv_.wait_for(...) to put the Worker Thread to sleep for 1 second,
    // while allowing it to wake up immediately when Stop() is called.
    std::mutex              mutex_;
    std::condition_variable cv_;

    // Declared last to ensure all other resources are fully initialized before the thread starts
    std::thread thread_;                 
};

#endif // MONITOR_WORKER_H

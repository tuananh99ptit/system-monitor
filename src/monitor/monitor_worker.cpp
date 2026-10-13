#include "monitor_worker.h"
#include <chrono>
#include <iostream>

// Constructor & Destructor
MonitorWorker::MonitorWorker(MonitorData& data) : data_(data) {

}

MonitorWorker::~MonitorWorker() {
    Stop();
}

// Start() Method (Activates the Thread)
void MonitorWorker::Start() {
    // exchange(true) returns the OLD value: if it was already true, it means it already started
    if (running_.exchange(true)) {
        return;
    }
    thread_ = std::thread(&MonitorWorker::Run, this);
}

// Stop() Method (Immediate Thread Termination)
void MonitorWorker::Stop() {
    {
        // Change the flag WHILE HOLDING the mutex to avoid a lost wakeup
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        cv_.notify_all();                    // Wake up the worker thread if it is currently sleeping
    }
    if (thread_.joinable()) {
        thread_.join();                  // Wait for the worker thread to completely exit
    }
}

// Run() Method (Main Loop of the Worker Thread)
void MonitorWorker::Run() {
    // Priming read: establishes the baseline T1 for CpuMonitor, discarding the initial result
    cpu_.GetCpuUsage();

    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, std::chrono::seconds(1), [this] { return !running_; });
        }
        if (!running_) {
            break;
        }

        // Heavy operations (reading /proc) are performed OUTSIDE of any locked regions
        SystemInfo info = Collect();
        data_.Update(info);              /// Update() handles its own internal MonitorData mutex locking
    }
}

// Collect() Method (Gathers & Packages Data)
SystemInfo MonitorWorker::Collect() {
    SystemInfo info;
    // CPU: Calls GetCpuUsage() to calculate the delta from the previous baseline.
    info.cpuUsage = cpu_.GetCpuUsage();

    MemoryInfo mem = memory_.GetMemoryInfo();
    info.memoryUsage = (mem.total > 0) ? static_cast<double>(mem.used) * 100.0 / static_cast<double>(mem.total) : 0.0;

    // Disk: Retrieves the hard drive usage percentage via GetDiskUsage().
    info.diskUsage = disk_.GetDiskUsage();

    // Process: Counts the number of active processes by checking the size of the GetProcesses() list.
    info.processCount = static_cast<int>(process_.GetProcesses().size());

    // Returns the fully packaged SystemInfo struct containing all 4 snapshot metrics.
    return info;
}
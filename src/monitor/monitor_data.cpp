#include "monitor_data.h"

void MonitorData::Update(const SystemInfo& info) {
    std::lock_guard<std::mutex> lock(mutex_);
    info_ = info;
}   // lock_guard is destroyed here => automatically unlocks

SystemInfo MonitorData::Get() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return info_;
    // Returns a COPY; the copying occurs while holding the lock
}
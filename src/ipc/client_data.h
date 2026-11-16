#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H

#include "monitor_data.h"
#include "protocol.h"
#include <mutex>

// Publicly inherit ClientData from MonitorData for compatibility with ConsoleRenderer
class ClientData : public MonitorData {
public:
    // Update data received from IPC Server and store it in the base MonitorData
    void update(const ipc::StatusPayload& payload) {
        SystemInfo info{};
        info.cpuUsage = payload.cpu;
        info.memoryUsage = payload.memory;
        info.diskUsage = payload.disk;
        info.processCount = static_cast<int>(payload.processCount);
        
        // Call MonitorData's Update function for thread-safe storage
        Update(info); 
        hasData_ = true;
    }

    // Check if the client has received the first data frame from the server
    bool hasData() const {
        return hasData_;
    }

private:
    bool hasData_ = false;
};

#endif // CLIENT_DATA_H

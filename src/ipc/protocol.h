#ifndef IPC_PROTOCOL_H
#define IPC_PROTOCOL_H

#include <cstdint>
#include <string>

namespace ipc {

// Maximum size limit for a single message (1 MB) to prevent memory overflow if the Header is corrupted
constexpr uint32_t MAX_MESSAGE_SIZE = 1024 * 1024;

// Data structure representing the Response message returned by the Server
struct StatusPayload {
    double cpu = 0.0;           // CPU usage percentage (%)
    double memory = 0.0;        // RAM usage percentage (%)
    double disk = 0.0 ;         // Disk usage percentage (%)
    double processCount = 0;    // Total number of running processes on the system
};

// Static utility class handling Request / Response protocol framing and socket transmission
class Protocol {
public:
    // =========================================================================
    // 1. MESSAGE FRAMING LAYER (PREVENTS PACKET STICKING / PARTIAL READS)
    // =========================================================================

    // Sends a complete JSON message: [4-byte length (htonl)] + [N-byte JSON string]
    static bool SendMessage(int sockFd, const std::string& jsonPayload);

    // Receives a complete JSON message from the Socket and stores it in outJsonPayload
    static bool ReceiveMessage(int sockFd, std::string& outJsonPayload);

    // =========================================================================
    // 2. SERIALIZATION & DESERIALIZATION LAYER (JSON REQUEST / RESPONSE)
    // =========================================================================

    // Client -> Server: Creates the JSON Request string {"type":"get_status"}
    static std::string CreateGetStatusRequest();

    // Generic function for all request types
    static std::string CreateRequest(const std::string& type);
    
    // Server: Parses the JSON Request string to extract the "type" field value (e.g., "get_status")
    static std::string ParseRequestType(const std::string& jsonRequest);

    // Server -> Client: Serializes StatusPayload into a JSON Response string
    // Example: {"cpu":25,"memory":60,"disk":40,"processCount":243}
    static std::string CreateStatusResponse(const StatusPayload& status);

    // Client: Deserializes the JSON Response string into a StatusPayload struct
    static bool ParseStatusRespone(const std::string& jsonResponse, StatusPayload& outStatus);

private:

    // Ensures exact 'length' bytes are sent down to the Kernel (handles partial writes & EINTR)
    static bool SendAll(int sockFd, const void* buffer, size_t length);

    // Ensures exact 'length' bytes are read from the Kernel (handles partial reads & EINTR)
    static bool RecvAll(int sockFd, void* buffer, size_t length);
};
} // namespace ipc 

#endif // IPC_PROTOCOL_H

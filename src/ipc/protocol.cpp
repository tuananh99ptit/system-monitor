#include "protocol.h"

#include <arpa/inet.h>          // Provides htonl() and ntohl() to standardize Network Byte Order
#include <cerrno>               // Provides the errno variable and EINTR error code
#include <cstring>              // Provides std::memcpy
#include <iostream>             // Provides std::cerr for error logging
#include <sstream>              // Provides std::ostringstream for high-performance JSON string creation
#include <sys/socket.h>         // Provides send(), recv(), and the MSG_NOSIGNAL flag
#include <vector>               // Provides std::vector for dynamic buffer allocation when reading packets

namespace ipc {

    // ==============================================================================
    // 1. DATA FRAMING LAYER (FRAMING LAYER)
    // ==============================================================================

    bool Protocol::SendAll(int sockFd, const void* buffer, size_t length) {

        // Cast to char* to advance byte by byte
        const char* ptr = static_cast<const char*>(buffer);

        // Total number of bytes successfully sent
        size_t totalSent = 0;
        
        while (totalSent < length) {

            // MSG_NOSIGNAL: Prevents the OS from raising a SIGPIPE signal that crashes the process if the peer disconnects
            ssize_t sent = send(sockFd, ptr + totalSent, length - totalSent, MSG_NOSIGNAL);
            if(sent < 0) {
                // Temporarily interrupted by a system Signal -> retry sending
                if(errno == EINTR) continue;
                return false;
            }
            if(sent == 0) {
                // Transmission channel has been closed
                return false;
            }
            totalSent = totalSent + static_cast<size_t>(sent);      // Accumulate the number of bytes sent
        }
        return true;
    }

    bool Protocol::RecvAll(int sockFd, void* buffer, size_t length) {

        // Cast to char* to advance byte by byte
        char* ptr = static_cast<char*>(buffer);

        // Total number of bytes received
        size_t totalReceived = 0;

        while(totalReceived < length) {
            ssize_t received = recv(sockFd, ptr + totalReceived, length - totalReceived, 0);
            if(received < 0) {
                // Interrupted by a Signal -> continue reading
                if(errno == EINTR) continue;
                return false;
            }
            if(received == 0) {
                // Peer has closed the connection
                return false;
            }
            totalReceived = totalReceived + static_cast<size_t>(received);  // Accumulate the number of bytes just received
        }
        return true;
    }

    bool Protocol::SendMessage(int sockFd, const std::string& jsonPayload) {
        if(sockFd < 0 || jsonPayload.empty()) return false;
        if(jsonPayload.size() > MAX_MESSAGE_SIZE) {
            std::cerr << "[Protocol Error] Message size exceeds the allowed limit!\n";
            return false;
        }

        // Step 1: Convert the JSON string length to Network Byte Order (4-byte Big-Endian)
        uint32_t netLength = htonl(static_cast<uint32_t>(jsonPayload.size()));

        // Send the 4-byte length Header first
        // Fails if Client/Server disconnects
        if(!SendAll(sockFd, &netLength, sizeof(netLength))) {
            return false;
        }

        // Step 2: Send the entire JSON Payload string content
        return SendAll(sockFd, jsonPayload.data(), jsonPayload.size());
    }

    bool Protocol::ReceiveMessage(int sockFd, std::string& outJsonPayload) {
        if(sockFd < 0) return false;

        // Step 1: Read the fixed first 4-byte Header to determine the packet size
        uint32_t netLength = 0;
        // Fails if Client/Server disconnects
        if(!RecvAll(sockFd, &netLength, sizeof(netLength))) {
            return false;
        }

        // Convert from Network Byte Order back to the CPU's Host Byte Order
        uint32_t payloadLength = ntohl(netLength);
        if(payloadLength == 0 || payloadLength > MAX_MESSAGE_SIZE) {
            std::cerr << "[Protocol Error] Invalid message length: " << payloadLength << "\n";
            return false;
        }

        // Step 2: Allocate a buffer sized to fit payloadLength and read the entire JSON string
        std::vector<char> buffer(payloadLength);
        // Fails if Client/Server disconnects
        if(!RecvAll(sockFd, buffer.data(), payloadLength)) {
            return false;
        }

        // Assign the received data to the output string
        outJsonPayload.assign(buffer.begin(), buffer.end());
        return true;
    }

    // ==============================================================================
    // 2. SERIALIZATION & DESERIALIZATION LAYER (JSON)
    // ==============================================================================

    // Generic function for all request types
    std::string Protocol::CreateRequest(const std::string& type) {
        std::ostringstream oss;
        oss << R"({"type":")" << type << R"("})";
        return oss.str();
    }

    std::string Protocol::CreateGetStatusRequest() {
        // Returns the JSON Request string compliant with the Task 6 specification
        return CreateRequest("get_status");
    }

    std::string Protocol::ParseRequestType(const std::string& jsonRequest) {
        // Find the position of the "type" key in the JSON string
        const std::string key = "\"type\"";
        size_t keyPos = jsonRequest.find(key);
        if(keyPos == std::string::npos) return "";

        // Find the colon ':' after the "type" key
        size_t colonPos = jsonRequest.find(':', keyPos + key.length());
        if(colonPos == std::string::npos) return "";

        // Find the opening '"' and closing '"' quotes of the value (e.g., "get_status")
        size_t quoteStart = jsonRequest.find('"', colonPos + 1);
        if(quoteStart == std::string::npos) return "";

        size_t quoteEnd = jsonRequest.find('"', quoteStart + 1);
        if(quoteEnd == std::string::npos) return "";

        // Extract the string located between the two quotation marks
        return jsonRequest.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
    }

    std::string Protocol::CreateStatusResponse(const StatusPayload& status) {
        // Use std::ostringstream to serialize the struct into a standard JSON string
        std::ostringstream oss;
        oss << "{"
        << "\"cpu\":" << status.cpu << ","
        << "\"memory\":" << status.memory << ","
        << "\"disk\":" << status.disk << ","
        << "\"processCount\":" << status.processCount
        << "}";
        return oss.str();
    }

namespace {
// Internal helper function to extract a numeric value by key name from a flat JSON string
bool ExtractNumberField(const std::string& json, const std::string& key, double& outValue) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if(keyPos == std::string::npos) return false;

    size_t colonPos = json.find(':', keyPos + searchKey.length());
    if(colonPos == std::string::npos) return false;

    // Skip whitespace after the ':' character if present
    size_t valStart = json.find_first_not_of(" \t\n\r", colonPos + 1);
    if(valStart == std::string::npos) return false;
    try {
        // std::stod automatically parses the numeric portion and stops upon encountering ',' or '}'
        outValue = std::stod(json.substr(valStart));
        return true;
    } catch(...) {
        return false;
    }
}
} // namespace 

bool Protocol::ParseStatusRespone(const std::string& jsonResponse, StatusPayload& outStatus) {
    if(jsonResponse.empty() || jsonResponse.front() != '{') {
        return false;
    }

    double cpu = 0.0;
    double memory = 0.0;
    double disk = 0.0;
    double procCount = 0.0;

    // Extract all 4 required fields according to the Response specification
    if( !ExtractNumberField(jsonResponse, "cpu", cpu) || !ExtractNumberField(jsonResponse, "memory", memory) ||
        !ExtractNumberField(jsonResponse, "disk", disk) || !ExtractNumberField(jsonResponse, "processCount", procCount)) {
            return false;
        }
    
    outStatus.cpu = cpu;
    outStatus.memory = memory;
    outStatus.disk = disk;
    outStatus.processCount = static_cast<int>(procCount);
    return true;    
}

} // namespace ipc

// Use test_protocol.cpp to test Request JSON / Response JSON

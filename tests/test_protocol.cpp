#include "protocol.h"
#include <cassert>
#include <iostream>

int main() {
    // 1. Test Request Serialization & Parsing
    std::string request = ipc::Protocol::CreateGetStatusRequest();
    std::cout << "[Request JSON]:  " << request << "\n";
    assert(ipc::Protocol::ParseRequestType(request) == "get_status");

    // 2. Test Response Serialization & Deserialization
    ipc::StatusPayload original{25.0, 60.0, 40.0, 243};
    std::string response = ipc::Protocol::CreateStatusResponse(original);
    std::cout << "[Response JSON]: " << response << "\n";

    ipc::StatusPayload parsed{};
    bool ok = ipc::Protocol::ParseStatusRespone(response, parsed);
    assert(ok);
    assert(parsed.cpu == 25.0);
    assert(parsed.memory == 60.0);
    assert(parsed.disk == 40.0);
    assert(parsed.processCount == 243);

    std::cout << "[PASS] All Protocol Request/Response tests passed!\n";
    return 0;
}

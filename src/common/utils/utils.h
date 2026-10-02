#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace Utils {
    std::vector<std::string> Split(const std::string& str, char delimiter = ' ');
    std::string Trim(const std::string& str);
    double BytesToGB(uint64_t bytes);
    double KBToGB(uint64_t kb);
}
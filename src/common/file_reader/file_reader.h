#pragma once
#include <string>
#include <vector>

class FileReader {
public:
    static std::vector<std::string> ReadLines(const std::string& filePath);
    static std::string ReadFirstLine(const std::string& filePath);
};
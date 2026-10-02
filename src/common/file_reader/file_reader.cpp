#include "file_reader.h"
#include "logger.h"
#include <fstream>

std::vector<std::string> FileReader::ReadLines(const std::string& filePath) {
    std::vector<std::string> lines;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        Logger::Error("Fail to open file: " + filePath);
        return lines;
    }
    std::string line;
    while(std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}


std::string FileReader::ReadFirstLine(const std::string& filePath) {
    std::ifstream file(filePath);
    if(!file.is_open()) {
        Logger::Error("Fail to open file: " + filePath);
        return "";
    }
    std::string line;
    std:: getline(file, line);
    return line;
}

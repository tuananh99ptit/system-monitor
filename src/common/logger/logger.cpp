#include "logger.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>

std::string Logger::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_r(&time_t_now, &tm_buf);

    std::ostringstream ss;
    ss << std::put_time(&tm_buf, "%H:%M:%S");
    return ss.str();
}

void Logger::WriteLog(const std::string& level, const std::string& message) {
    std::string timestamp = GetCurrentTimestamp();
    std::string formattedMessage = timestamp + " " + level + " " + message;
    if (level == "ERROR") {
        std::cerr << formattedMessage << std::endl;
    }
    else {
        std::cout << formattedMessage << std::endl;
    }

    std::ofstream logFile(Config::LOG_FILE_PATH, std::ios::app);
    if(logFile.is_open()) {
        logFile << formattedMessage << "\n";
    }
}

void Logger::Info(const std::string& message) {
    WriteLog("INFO", message);
}
void Logger::Warn(const std::string& message) {
    WriteLog("WARN", message);
}
void Logger::Error(const std::string& message) {
    WriteLog("ERROR", message);
}


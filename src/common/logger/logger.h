#pragma once
#include <string>

class Logger {
public:
    static void Info(const std::string& message);
    static void Warn(const std::string& message);
    static void Error(const std::string& message);

private:
    static void WriteLog(const std::string& level, const std::string& message);
    static std::string GetCurrentTimestamp();
};
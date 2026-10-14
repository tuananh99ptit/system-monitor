#include "console_renderer.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace {
    // ANSI escape codes
    // \033[K : Clear from the current cursor position to the end of the line (prevents artifacts from the previous frame)
    constexpr const char* kClearEol = "\033[K";

    // PROGRESS BAR DRAWING FUNCTIONS [#####---------------]
std::string MakeBar(double percent, int width = 20){
    // Clamp values outside the% range

        if(percent < 0.0) percent = 0.0;
        if(percent > 100) percent = 100.0;
    // Calculate the number of '#' characters corresponding to the percentage
        int filled = static_cast<int>(percent / 100.0 * width + 0.5);
        return "[" + std::string(filled, '#') + std::string(width - filled, '-') + "]";
    }

    // REAL-TIME GETTER FUNCTIONS (THREAD-SAFE)
std::string NowString() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_r(&t, &tm_buf);       //Use localtime_r for thread safety in multi-threaded environments

    std::ostringstream ss;
    ss << std::put_time(&tm_buf, "%H:%M:%S");
    return ss.str();
}
}

ConsoleRenderer::ConsoleRenderer(const MonitorData& data) : data_(data) {}


std::string ConsoleRenderer::BuildFrame(const SystemInfo& info) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1);

    auto percentLine = [&](const char* label, double value) {
        out << std::left << std::setw(15) << label << ": "
            << std::right << std::setw(5) << value << "% "
            << MakeBar(value) << kClearEol << "\n";
    };

    out << "============================================================" << kClearEol << "\n";
    out << "         SYSTEM MONITOR"           << kClearEol << "\n";
    out << "============================================================" << kClearEol << "\n";
    out << kClearEol << "\n";

    percentLine("CPU Usage",    info.cpuUsage);
    percentLine("Memory Usage", info.memoryUsage);
    percentLine("Disk Usage",   info.diskUsage);

    out << std::left << std::setw(15) << "Process Count" << ": "
        << info.processCount << kClearEol << "\n";
    out << "============================================================" << "\n";
    return out.str();
}

// RENDER FUNCTIONS (FLICKER-FREE SCREEN PRINTING)
void ConsoleRenderer::Render() {
    // Take a data snapshot (Fast operation, holding the mutex for a few microseconds)
    SystemInfo info = data_.Get();
    // Build the entire frame into a single string in memory
    std::string frame = BuildFrame(info);
    frame += "\nRendered at " + NowString() + kClearEol + "\n";
    // Print to the screen in a single operation 
    if (firstFrame_) {
        std::cout << "\033[2J";     // Clear the entire screen exactly once on the first frame
        firstFrame_ = false;
    }
     std::cout << "\033[H"          // \033[H : Move the cursor to the top-left corner (without clearing the screen)
              << frame              // Print the new frame content, overwriting the old one
              << "\033[J"           // \033[J : Clear remaining lines below (if the new frame is shorter)
              << std::flush;        // Flush data to the terminal immediately
}

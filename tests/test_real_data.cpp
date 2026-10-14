#include "monitor_worker.h"
#include "console_renderer.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

static int g_failed = 0; // Counter for each metric, incremented upon failure

static void Check(bool condition, const char* name) {
    if (condition) {
        std::cout << "[PASS] " << name << "\n";
    } else {
        std::cout << "[FAIL] " << name << "\n";
        ++g_failed;
    }
}

static bool Contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

// ===== Reference sources: calculated using an ALTERNATIVE METHOD from the code under test =====

// Memory usage (%): Read directly from /proc/meminfo
static double ReferenceMemoryPercent() {
    std::ifstream file("/proc/meminfo");
    std::string line;
    long total = 0, available = 0, value = 0;
    while (std::getline(file, line)) {
        if (std::sscanf(line.c_str(), "MemTotal: %ld", &value) == 1) total = value;
        else if (std::sscanf(line.c_str(), "MemAvailable: %ld", &value) == 1) available = value;
    }
    return total > 0 ? 100.0 * (total - available) / total : 0.0;
}

// Disk usage (%): Equivalent to the 'df' command, using std::filesystem
static double ReferenceDiskPercent() {
    std::filesystem::space_info s = std::filesystem::space("/");
    double used = static_cast<double>(s.capacity - s.free);
    double denominator = used + static_cast<double>(s.available);
    return denominator > 0 ? 100.0 * used / denominator : 0.0;
}

// Process count: Count directories containing only digits inside /proc
static int ReferenceProcessCount() {
    int count = 0;
    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        std::string name = entry.path().filename().string();
        bool allDigits = !name.empty() &&
            std::all_of(name.begin(), name.end(),
                        [](unsigned char c) { return std::isdigit(c); });
        if (entry.is_directory() && allDigits) {
            ++count;
        }
    }
    return count;
}

int main() {
    MonitorData data;                          
    MonitorWorker worker(data);

    // Before Worker runs: Data must be at its default values
    Check(data.Get().processCount == 0, "Before start: default data");

    // Run the actual Worker, wait for at least 2 updates
    worker.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    SystemInfo info = data.Get();

    // Retrieve reference metrics IMMEDIATELY AFTER reading to minimize time drift
    double refMem  = ReferenceMemoryPercent();
    double refDisk = ReferenceDiskPercent();
    int    refProc = ReferenceProcessCount();
    worker.Stop();

    // Print metrics for visual inspection in case of a FAIL
    std::cout << "Measured : CPU=" << info.cpuUsage << " MEM=" << info.memoryUsage
              << " DISK=" << info.diskUsage << " PROC=" << info.processCount << "\n";
    std::cout << "Reference: MEM=" << refMem << " DISK=" << refDisk
              << " PROC=" << refProc << "\n";

    //CPU: Only verify valid range (value changes constantly)
    Check(info.cpuUsage >= 0.0 && info.cpuUsage <= 100.0, "CPU in range 0..100");

    // Memory: In range and matches /proc/meminfo (5% tolerance)
    Check(info.memoryUsage > 0.0 && info.memoryUsage <= 100.0, "Memory in range 0..100");
    Check(std::abs(info.memoryUsage - refMem) <= 5.0, "Memory matches /proc/meminfo");

    // Disk: In range and matches df (1% tolerance, disk changes very slowly)
    Check(info.diskUsage > 0.0 && info.diskUsage <= 100.0, "Disk in range 0..100");
    Check(std::abs(info.diskUsage - refDisk) <= 1.0, "Disk matches df");

    // Process: > 0 and matches /proc (50 tolerance, processes are created/destroyed continuously)
    Check(info.processCount > 0, "Process count > 0");
    Check(std::abs(info.processCount - refProc) <= 50, "Process count matches /proc");

    // Real data passed into the renderer should still render correctly
    std::string frame = ConsoleRenderer::BuildFrame(info);
    Check(Contains(frame, "SYSTEM MONITOR"), "Real data: has title");
    Check(Contains(frame, std::to_string(info.processCount)), "Real data: shows process count");

    return g_failed == 0 ? 0 : 1;
}
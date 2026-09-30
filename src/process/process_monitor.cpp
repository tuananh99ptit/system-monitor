#include "process_monitor.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cctype> // std::isdigit

std::string ProcessMonitor::ConVertStateChar(char stateChar)
{
    switch (stateChar)
    {
    case 'R':
        return "RUN";
    case 'S':
        return "SLEEP";
    case 'D':
        return "DISK_SLEEP"; // Uninterruptible sleep, thường do chờ I/O
    case 'I':
        return "IDLE"; // Kernel thread đang rảnh, không có việc để làm
    case 'Z':
        return "ZOMBIE";
    case 'T':
        return "STOP";
    case 't':
        return "TRACE_STOP";
    case 'X':
    case 'x':
        return "DEAD";
    default:
        return "UNKNOWN";
    }
}

bool ProcessMonitor::ReadProcessStat(int pid, ProcessInfo &outInfo)
{
    std::string statPath = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream file(statPath);

    if (!file.is_open())
    {
        // Tiến trình có thể đã kết thúc đúng lúc ta mở file (race condition)
        return false;
    }

    // Đọc toàn bộ dòng duy nhất trong file /proc/[pid]/stat
    std::string line;
    std::getline(file, line);

    // Tìm dấu '(' ĐẦU TIÊN và ')' CUỐI CÙNG để lấy đúng tên tiến trình,
    // vì tên có thể chứa khoảng trắng hoặc dấu ngoặc bên trong (xem lý thuyết)
    size_t firstParen = line.find('(');
    size_t lastParen = line.rfind(')');

    if (firstParen == std::string::npos || lastParen == std::string::npos || lastParen < firstParen)
    {
        return false; // Định dạng dòng bất thường, bỏ qua an toàn
    }

    // Tên tiến trình nằm giữa 2 dấu ngoặc đó
    std::string name = line.substr(firstParen + 1, lastParen - firstParen - 1);

    // Sau dấu ')' cuối cùng, cách 1 khoảng trắng, là ký tự trạng thái
    // Ví dụ: "...) S 1000 ..." -> lastParen trỏ vào ')', +2 để nhảy qua ") "
    char stateChar = line[lastParen + 2];

    outInfo.pid = pid;
    outInfo.name = name;
    outInfo.state = ConVertStateChar(stateChar);

    return true;
}

std::vector<ProcessInfo> ProcessMonitor::GetProcesses()
{
    std::vector<ProcessInfo> processes;

    // Duyệt tất cả entry (file/thư mục) bên trong /proc bằng std::filesystem
    // (thư viện chuẩn C++17, thay thế cách cũ dùng opendir()/readdir() của C)
    for (const auto &entry : std::filesystem::directory_iterator("/proc"))
    {
        if (!entry.is_directory())
        {
            continue; // Bỏ qua, chỉ quan tâm thư mục (mỗi PID là 1 thư mục)
        }

        std::string folderName = entry.path().filename().string();

        // /proc còn chứa nhiều thư mục KHÔNG phải PID, ví dụ "self", "net",
        // "sys"... Chỉ những thư mục có tên TOÀN BỘ LÀ CHỮ SỐ mới là PID thật
        bool isAllDigits = !folderName.empty() &&
                            std::all_of(folderName.begin(), folderName.end(),
                                        [](unsigned char c)
                                        { return std::isdigit(c); });

        if (!isAllDigits)
        {
            continue;
        }

        int pid = std::stoi(folderName);

        ProcessInfo info;
        if (ReadProcessStat(pid, info))
        {
            processes.push_back(info);
        }
        // Nếu ReadProcessStat trả về false (tiến trình vừa kết thúc), bỏ qua
        // an toàn, không thêm vào danh sách, không báo lỗi (đây là bình thường)
    }

    return processes;
}
#include "memory_monitor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
long MemoryMonitor::GetValueByLabel(const std::string &content, const std::string &label)
{
    // Tìm vị trí của nhãn (ví dụ "MemTotal:") trong toàn bộ nội dung file
    size_t pos = content.find(label);
    if (pos == std::string::npos)
    {
        return 0; // Không tìm thấy nhãn -> trả về 0
    }

    // Dịch con trỏ đến sau nhãn để bắt đầu đọc số
    pos += label.length();

    // Dùng stringstream để tách số nguyên đầu tiên sau nhãn (bỏ qua "kB")
    std::istringstream iss(content.substr(pos));
    long value = 0;
    iss >> value;

    return value; // Đơn vị KB
}

MemoryInfo MemoryMonitor::GetMemoryInfo()
{
    MemoryInfo info{0, 0, 0};

    // Mở file hệ thống /proc/meminfo
    std::ifstream file("/proc/meminfo");
    if (!file.is_open())
    {
        std::cerr << "Khong the mo file /proc/meminfo" << std::endl;
        return info;
    }

    // Đọc toàn bộ nội dung file vào một chuỗi để dễ tìm kiếm theo nhãn
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Lấy các giá trị cần thiết (đơn vị KB)
    long memTotal = GetValueByLabel(content, "MemTotal:");
    long memAvailable = GetValueByLabel(content, "MemAvailable:");

    // Nếu kernel không hỗ trợ MemAvailable (kernel cũ), tự tính gần đúng
    // bằng MemFree + Buffers + Cached + SReclaimable
    if (memAvailable == 0)
    {
        long memFree = GetValueByLabel(content, "MemFree:");
        long buffers = GetValueByLabel(content, "Buffers:");
        long cached = GetValueByLabel(content, "Cached:");

        // SReclaimable: phần bộ nhớ Slab (dentry/inode cache) có thể thu hồi
        // ngay khi hệ thống cần RAM, nên phải cộng vào phần "khả dụng"
        long sReclaimable = GetValueByLabel(content, "SReclaimable:");

        memAvailable = memFree + buffers + cached + sReclaimable;
    }

    // Gán kết quả: Used = Total - Available
    info.total = memTotal;
    info.free = memAvailable;
    info.used = memTotal - memAvailable;

    return info;
}
// Hàm tiện ích: chuyển đổi từ KB sang GB để hiển thị
double KbToGb(long kb)
{
    return static_cast<double>(kb) / (1024.0 * 1024.0);
}

int main()
{
    MemoryMonitor monitor;
    while(true) {
    // Lấy thông tin bộ nhớ hiện tại của hệ thống
    MemoryInfo info = monitor.GetMemoryInfo();

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "+-----------------------------+\n";
    std::cout << "| Total Memory : " << KbToGb(info.total) << " GB\n";
    std::cout << "| Used Memory  : " << KbToGb(info.used) << " GB\n";
    std::cout << "| Free Memory  : " << KbToGb(info.free) << " GB\n";
    std::cout << "+-----------------------------+\n";
    }
    return 0;
}
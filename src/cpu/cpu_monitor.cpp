#include "cpu_monitor.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
// constructor : khoi tao cac moc thoi gian ban dau deu bang khong 
CpuMonitor::CpuMonitor() {
    prevIdleTime = 0;
    prevTotalTime = 0;
}
float CpuMonitor::GetCpuUsage() {
    // mo file he thong /proc/stat
    std::ifstream file("/proc/stat");
    if(!file.is_open()){
        return 0.0f;
    }
    // doc dong dau tien chua tong quan du lieu cpu
    std::string line;
    std::getline(file,line);
    file.close();
    // phan tach dong chu thanh tung so nguyen 
    std::istringstream ss(line);
    std::string CpuLabel;
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    // doc CpuLabel voi 8 tra tri nhip cpu tuong ung 
    ss >> CpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal ;
    // 4. Tính toán IdleTime và TotalTime ở thời điểm hiện tại (T2)
    unsigned long long idleTime = idle + iowait;
    unsigned long long totalTime = user + nice + system + idle + iowait + irq + softirq + steal;
    // tinh do chenh lech delta giua lan doc nay so voi lan truoc
    unsigned long long totalDiff = totalTime - prevTotalTime;
    unsigned long long idleDiff = idleTime - prevIdleTime;
    //Cập nhật mốc thời gian cũ để dùng cho lần gọi GetCpuUsage() tiếp theo
    prevTotalTime = totalTime;
    prevIdleTime = idleTime;

    // Tránh lỗi chia cho 0 nếu gọi hàm 2 lần liên tiếp quá nhanh
    if (totalDiff == 0) {
        return 0.0f;
    }
    //Áp dụng công thức % CPU = (1 - ΔIdle / ΔTotal) * 100
    double usageRatio = 1.0 - static_cast<double>(idleDiff) / totalDiff;
    return static_cast<float>(usageRatio * 100.0);
}
int main() {
    CpuMonitor monitor;

    // Chuẩn bị mốc thời gian ban đầu trước khi vào vòng lặp
    monitor.GetCpuUsage(); 

    while (true) {
        // 1. Chờ 500ms để hệ thống tích lũy chênh lệch thời gian
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // 2. Tính % CPU thực tế dựa trên chênh lệch với lần đọc trước
        float cpu = monitor.GetCpuUsage();

        // 3. Xóa màn hình và đưa con trỏ về góc trên cùng
        std::cout << "\033[2J\033[1;1H";

        // 4. In kết quả 
        std::cout << "=== LINUX SYSTEM MONITOR ===" << std::endl;
        std::cout << "+----------------------+\n";
        std::cout << "| CPU Usage : " << std::setw(3) << static_cast<int>(cpu) << "%       |\n";
        std::cout << "+----------------------+\n";
    }

    return 0;
}
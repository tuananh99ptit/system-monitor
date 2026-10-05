#include "app/app.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>


void App::Display() {
    auto info = monitor_.GetMemoryInfo();
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "+-----------------------------+\n";
    std::cout << "| Total Memory : " << KbToGb(info.total) << " GB\n";
    std::cout << "| Used Memory  : " << KbToGb(info.used) << " GB\n";
    std::cout << "| Free Memory  : " << KbToGb(info.free) << " GB\n";
    std::cout << "+-----------------------------+\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void App::Run() {
    while (true) {
        Display();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

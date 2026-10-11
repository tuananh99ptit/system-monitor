#include "producer.h"
#include <chrono>
#include <iostream>

Producer::Producer(ThreadSafeQueue<std::string>& queue) : queue_(queue) {}

Producer::~Producer() { Stop(); }

void Producer::Start() {
    if (running_.exchange(true)) {
        return;                  // đã chạy rồi thì bỏ qua
    }
    thread_ = std::thread(&Producer::Run, this);
}

void Producer::Stop() {
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void Producer::Run() {
    cpu_.GetCpuUsage();          // priming read, giống App::Run()
    std::this_thread::sleep_for(std::chrono::seconds(1));

    while (running_) {
        std::string msg = "CPU=" + std::to_string(static_cast<int>(cpu_.GetCpuUsage())) + "%";
        std::cout << "Producer -> " + msg + "\n";
        queue_.Push(msg);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
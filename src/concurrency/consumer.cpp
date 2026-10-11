#include "consumer.h"
#include <iostream>

Consumer::Consumer(ThreadSafeQueue<std::string>& queue) : queue_(queue) {}

Consumer::~Consumer() { Join(); }

void Consumer::Start() {
    thread_ = std::thread(&Consumer::Run, this);
}

void Consumer::Join() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

void Consumer::Run() {
    // Pop() trả nullopt khi queue đã Close() và hết dữ liệu -> thoát vòng lặp
    while (auto msg = queue_.Pop()) {
        std::cout << "Consumer <- " + *msg + "\n";
    }
}
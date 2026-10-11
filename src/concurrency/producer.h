#ifndef PRODUCER_H
#define PRODUCER_H

#include "cpu_monitor.h"
#include "thread_safe_queue.h"
#include <atomic>
#include <string>
#include <thread>

class Producer {
public:
    explicit Producer(ThreadSafeQueue<std::string>& queue);
    ~Producer();                 // Automatically calls Stop() to ensure the thread is always joined

    void Start();
    void Stop();

private:
    void Run();                  // Target function running inside the worker thread

    ThreadSafeQueue<std::string>& queue_;
    CpuMonitor cpu_;
    std::thread thread_;
    std::atomic<bool> running_{false};
};

#endif // PRODUCER_H

#ifndef CONSUMER_H
#define CONSUMER_H

#include "thread_safe_queue.h"
#include <string>
#include <thread>

class Consumer {
public:
    explicit Consumer(ThreadSafeQueue<std::string>& queue);
    ~Consumer();

    void Start();
    void Join();

private:
    void Run();

    ThreadSafeQueue<std::string>& queue_;
    std::thread thread_;
};

#endif // CONSUMER_H
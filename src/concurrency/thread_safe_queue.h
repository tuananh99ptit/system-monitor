#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

// Thread-safe queue for multi-threaded environments (header-only implementation for template)
template <typename T>
class ThreadSafeQueue {
public:
    void Push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cv_.notify_one();   // Wake up one waiting consumer thread
    }

    // Blocks until data becomes available.
    // Returns std::nullopt if the queue is closed and no elements remain.
    std::optional<T> Pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        // Wait re-evaluates the condition upon waking up to prevent spurious wakeups
        cv_.wait(lock, [this] { return !queue_.empty() || closed_; });

        if (queue_.empty()) {
            return std::nullopt;
        }
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    // Signals that "no more new data will be added" allowing consumers to exit their loops gracefully
    void Close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        cv_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool closed_ = false;
};

#endif // THREAD_SAFE_QUEUE_H

#include "signal_handler.h"
#include <algorithm>
#include <atomic>
#include <csignal>
#include <thread>
#include <unistd.h> // Used for _exit()

namespace {

// Global atomic variable storing the received signal code (0 = no signal received)
std::atomic<int> g_signal{0};

// Ensure std::atomic<int> is lock-free at compile-time
static_assert(std::atomic<int>::is_always_lock_free, "atomic<int> must be lock-free to be used in a signal handler");

// Asynchronous handler function invoked by the kernel upon process interruption
void Handler(int signalNumber) {
    // Ignore SIGHUP (Signal 1) emitted by VS Code / VMware PTY during window operations
    if (signalNumber == SIGHUP) {
        return;
    }

    // g_signal.exchange() assigns the new value and returns the OLD value.
    // If the old value != 0, it means the user pressed Ctrl+C a second time while the app is shutting down.
    // -> Force immediate termination via safe _exit().
    if (g_signal.exchange(signalNumber) != 0) {
        _exit(128 + signalNumber);
    }
}

} // namespace

namespace ShutdownSignal {

bool Install() {
    // 1. Configure kernel to completely IGNORE SIGHUP (Signal 1)
    std::signal(SIGHUP, SIG_IGN);

    // 2. Use standard POSIX sigaction struct to register SIGINT (2) and SIGTERM (15)
    struct sigaction action {};
    action.sa_handler = Handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0; // Intentionally avoid SA_RESTART to interrupt blocked system calls
    return sigaction(SIGINT, &action, nullptr) == 0 && sigaction(SIGTERM, &action, nullptr) == 0;
}

bool IsRequested() {
    return g_signal.load() != 0;
}

int LastSignal() {
    return g_signal.load();
}

void Reset() {
    g_signal.store(0);
}

bool SleepUnlessRequested(std::chrono::milliseconds total) {
    using Clock = std::chrono::steady_clock;
    constexpr std::chrono::milliseconds kSlice{50}; // Maximum response latency of 50ms
    const auto deadline = Clock::now() + total;

    while (!IsRequested()) {
        auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - Clock::now());
        if (remaining <= std::chrono::milliseconds::zero()) {
            break;
        }
        // Sleep in small 50ms increments
        std::this_thread::sleep_for(std::min(kSlice, remaining));
    }

    return IsRequested();
}

} // namespace ShutdownSignal

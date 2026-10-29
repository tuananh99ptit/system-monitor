#include "signal_handler.h"
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
#include <unistd.h>

using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

static int g_failed = 0;

static void Check(bool condition, const char* name) {
    if (condition) {
        std::cout << "[PASS] " << name << "\n";
    } else {
        std::cout << "[FAIL] " << name << "\n";
        ++g_failed;
    }
}

static long ElapsedMs(Clock::time_point start) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               Clock::now() - start).count();
}

int main() {
    // Check initial state
    Check(!ShutdownSignal::IsRequested(), "Initially no shutdown requested");
    Check(ShutdownSignal::LastSignal() == 0, "Initially no signal recorded");

    Check(ShutdownSignal::Install(), "Install handlers");

    // Test 1: Simulate SIGTERM
    std::raise(SIGTERM);
    Check(ShutdownSignal::IsRequested(), "SIGTERM sets the flag");
    Check(ShutdownSignal::LastSignal() == SIGTERM, "Remembers SIGTERM (15)");

    // Test 2: Reset and test SIGINT
    ShutdownSignal::Reset();
    Check(!ShutdownSignal::IsRequested(), "Reset clears the flag");
    std::raise(SIGINT);
    Check(ShutdownSignal::LastSignal() == SIGINT, "Remembers SIGINT (2)");

    // Test 3: Sleep without any signal -> Must sleep for the full 300ms
    ShutdownSignal::Reset();
    auto t0 = Clock::now();
    bool requested = ShutdownSignal::SleepUnlessRequested(300ms);
    long ms = ElapsedMs(t0);
    Check(!requested, "Sleep without signal returns false");
    Check(ms >= 280 && ms < 600, "Sleep lasts about the full duration");

    // Test 4: Raise a signal from another thread while sleeping -> Must wake up early
    ShutdownSignal::Reset();
    std::thread sender([] {
        std::this_thread::sleep_for(200ms);
        kill(getpid(), SIGTERM); // Equivalent to running 'kill -TERM' from an external terminal
    });

    t0 = Clock::now();
    requested = ShutdownSignal::SleepUnlessRequested(5000ms);
    ms = ElapsedMs(t0);
    sender.join();

    Check(requested, "Sleep returns true when signal arrives");
    Check(ms < 1000, "Sleep wakes up early (well before 5s)");

    return g_failed == 0 ? 0 : 1;
}

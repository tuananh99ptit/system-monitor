#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <chrono>

namespace ShutdownSignal {
    // Registers the signal handler for SIGINT and SIGTERM with the Linux kernel.
    // Returns true if the registration is successful.
    bool Install();

    // Checks whether a shutdown signal has been requested/received.
    bool IsRequested();

    // Returns the code of the last received signal (2 for SIGINT, 15 for SIGTERM, 0 if none).
    int LastSignal();

    // Resets the signal flag state to 0 (primarily used for unit testing).
    void Reset();

    // Alternative to pure sleep_for.
    // Sleeps for up to 'total' duration, but wakes up immediately if a signal is received.
    // Returns true if a shutdown signal was requested.
    bool SleepUnlessRequested(std::chrono::milliseconds total);
} // namespace ShutdownSignal
#endif // SIGNAL_HANDLER_H

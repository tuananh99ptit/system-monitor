/*
"Object Owns Thread" Pattern:
Instead of having main() manually create a separate std::thread externally and pass a function into it,
we encapsulate both std::thread socketThread_ and the control flag std::atomic<bool> running_ inside the UnixSocketClient class.

- Encapsulation: External callers (main()) do not need to know the internal socket or loop mechanics; they simply call client.start() to run and client.stop() to shut down.
- Resource Safety (RAII): The destructor ~UnixSocketClient() automatically calls stop(), ensuring the worker thread is always reclaimed (join()) before the object is destroyed in memory, preventing std::terminate() caused by a thread still running after its owning object has died.

1.2. Separating the Communication Thread and the UI/Main Thread:
Socket functions such as connect(), send(), and recv() are Blocking Calls (waiting for the OS to process). If placed directly in main():
- When the Server responds slowly or is busy reading /proc, the user interface will freeze and fail to re-render frames.
- When the user presses Ctrl+C, the process cannot respond immediately because it is blocked on recv().
Solution: Offload the entire Connect -> Send Request -> Wait for Response -> Parse JSON cycle to a background socketThread_, leaving the Main Thread free to focus on rendering the UI and listening for shutdown signals.

1.3. Shared Data Synchronization (ClientData + std::mutex):
When split into two independent threads, the memory holding system metrics (ClientData) becomes a Shared Resource:
- socketThread_ acts as the Writer (continuously overwriting with fresh metrics every second).
- Main Thread acts as the Reader (reading metrics to render onto the screen).
Without synchronization locks, the Main Thread might read right when socketThread_ has only written half of the data,
causing a Data Race (undefined behavior). Using std::mutex combined with std::lock_guard inside ClientData ensures that only one thread accesses the data at any given time.
*/

#ifndef CLIENT_DATA_H 
#define CLIENT_DATA_H

#include "protocol.h"
#include <mutex>

// Client-side data storage class ensuring safety when multiple threads read/write concurrently (Thread-Safe)
class ClientData {
public:
    // Called from the Socket Thread: Updates with the latest data received from the Server
    void update(const ipc::StatusPayload& newData) {
        // Automatically locks the mutex on function entry and unlocks on exit (RAII)
        std::lock_guard<std::mutex> lock(mutex_);

        // Overwrite with the new CPU, Memory, Disk, and ProcessCount metrics
        data_ = newData;

        // Mark that at least one valid data packet has been received
        hasData_ = true;
    }

    // Called from the Main Thread (UI): Retrieves a copy of the current data to render on screen
    ipc::StatusPayload get() const {
        // Lock the mutex to prevent the Socket Thread from overwriting mid-read
        std::lock_guard<std::mutex> lock(mutex_);

        // Return a thread-safe copy of the data
        return data_;
    }

    // Function to check whether the Client has received its first packet from the Server
    bool hasData() const {
        // Lock the mutex to protect the hasData_ flag
        std::lock_guard<std::mutex> lock(mutex_);
        return hasData_;
    }

private:
    // The 'mutable' keyword allows locking the mutex even inside 'const' getter methods
    mutable std::mutex mutex_;

    // Memory buffer storing the latest system metrics
    ipc::StatusPayload data_{};

    // Flag indicating whether valid data is available yet
    bool hasData_{false};
};

#endif // CLIENT_DATA_H

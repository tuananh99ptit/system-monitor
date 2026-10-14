#ifndef CONSOLE_RENDERER_H
#define CONSOLE_RENDERER_H

#include "monitor_data.h"
#include <string>

    // Build the class responsible for the user interface (UI - Console)
    // Only holds a const reference to MonitorData (read-only, modification is not allowed).
class ConsoleRenderer {
public:
    // Requires MonitorData to be passed during initialization.
    // explicit to prevent implicit type conversions.
    explicit ConsoleRenderer(const MonitorData& data);

    // Read the latest snapshot and render a frame to the terminal.
    // Must only be called from a SINGLE thread (UI thread / main).
    void Render();

    // Static function to convert SystemInfo into a frame text string.
    // Isolating this function makes writing Unit Tests easy without depending on the Terminal.
    static std::string BuildFrame(const SystemInfo& info);
private:
    // Reference to the shared read-only data
    const MonitorData& data_;
    // Flag to mark the first frame (to clear the screen exactly once)
    bool firstFrame_ = true;
};

#endif //CONSOLE_RENDERER_H

#include "client_data.h"        
#include "console_renderer.h"   
#include "signal_handler.h"     
#include "unix_socket_client.h" 

#include <chrono>               
#include <iostream>             
#include <unistd.h>             

int main() {                    
    // 1. Install graceful shutdown signal handlers for the client process
    if (!ShutdownSignal::Install()) {
        std::cerr << "[Client Error] Failed to install signal handlers\n";
        return 1;
    }

    // Print client startup message with PID and PPID
    std::cout << "Monitor Client Started (pid=" << getpid() << ", ppid=" << getppid() << ")\n";

    // 2. Initialize ClientData Cache and the UnixSocketClient background thread for network communication
    ClientData clientData;
    UnixSocketClient client(clientData, "/tmp/system_monitor.sock");

    // Start the socket client thread; exit if it fails
    if (!client.start()) {
        std::cerr << "[Client Error] Failed to start UnixSocketClient\n";
        return 1;
    }

    // 3. Initialize ConsoleRenderer, passing clientData directly into the constructor
    ConsoleRenderer renderer(clientData);

    // 4. UI loop on Main Thread: check for data and call Render() in real-time every second
    while (!ShutdownSignal::IsRequested() && client.isRunning()) {
        if (clientData.hasData()) { // Check if the cache has received data from the Service
            renderer.Render();      // Call Render() without arguments to automatically fetch data and draw the UI
        } else {
            std::cout << "[Client] Waiting for data from Monitor Service...\n"; // Message when no data is available yet
        }

        // Main Thread sleeps for 1 second before repeating the next UI rendering cycle
        ShutdownSignal::SleepUnlessRequested(std::chrono::seconds(1));
    }

    // 5. Upon receiving a shutdown signal, gracefully stop the Client
    std::cout << "\n[Client] Stopping process...\n";
    client.stop(); // Gracefully stop the socket client thread

    std::cout << "Monitor Client stopped.\n"; 
    return 0;                                 
}

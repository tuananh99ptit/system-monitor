#include <iostream>
#include <unistd.h>

int main() {
    std::cout << "Monitor Client Started (pid=" << getpid() << ", ppid=" << getppid() << ")" << std::endl;

    // TODO Task 5: connect to the service via Unix Socket
    // TODO Task 7: receive data on a separate thread, render using ConsoleRenderer

    return 0;
}

#include <QApplication>
#include "main_window.h"

int main(int argc, char* argv[]) {
    // Initialize QApplication to manage the overall GUI lifecycle and events
    QApplication app(argc, argv);

    // Initialize the main window
    MainWindow window;

    // Display the window on the screen
    window.show();

    // Start the event loop to wait for user interactions
    return app.exec();
}

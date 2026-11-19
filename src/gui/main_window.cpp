#include "main_window.h"
#include "dashboard_widget.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(800, 600);
    setWindowTitle("System Monitor");

    // Integrate Task 2
    auto* dashboard = new DashboardWidget(this); // Set MainWindow as parent

    // Set Dashboard as the central widget
    setCentralWidget(dashboard);
}

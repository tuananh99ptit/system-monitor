#include "main_window.h"
#include "dashboard_widget.h"   // Include DashboardWidget definition
#include "top_toolbar.h"        // Include TopToolbar definition

#include <QDebug>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    resize(800, 600);                                       // Set initial window dimensions
    setWindowTitle("System Monitor");                       // Set window title

    auto* centralContainer = new QWidget(this);             // Central widget containing both the toolbar and dashboard
    auto* mainLayout = new QVBoxLayout(centralContainer);   // Vertical layout stacking TopToolbar above DashboardWidget

    toolbar_ = new TopToolbar(centralContainer);            // Initialize TopToolbar as a child of centralContainer
    dashboard_ = new DashboardWidget(centralContainer);     // Initialize DashboardWidget as a child of centralContainer

    mainLayout->addWidget(toolbar_, 0);                     // Place the toolbar at the top
    mainLayout->addWidget(dashboard_, 1);                   // Place the dashboard at the bottom
    centralContainer->setLayout(mainLayout);                // Set the layout for the container
    setCentralWidget(centralContainer);                     // Set the container as the central widget of QMainWindow

    connect(toolbar_, &TopToolbar::refreshClicked,          // When Refresh is clicked on TopToolbar
            this, &MainWindow::onRefreshClicked);           // Invoke the handler slot on MainWindow
}

void MainWindow::onRefreshClicked() {
    qDebug() << "Refresh requested";                        // Log message to verify signal/slot execution
    dashboard_->onRefreshRequested();                       // Fixed dashboard -> dashboard_ to call the proper member pointer
}

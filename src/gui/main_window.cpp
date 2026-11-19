#include "main_window.h"
#include "dashboard_widget.h"

#include <QAction>
#include <QToolBar>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Set initial window dimensions
    resize(800, 600);
    // Set window title
    setWindowTitle("System Monitor");

    // Integrate Task 2
    dashboard_ = new DashboardWidget(this); // Set MainWindow as parent

    // Set Dashboard as the central widget
    setCentralWidget(dashboard_);

    // Initialize the toolbar and connect signals/slots
    setupToolbar();
}

void MainWindow::setupToolbar() {
    // Initialize the toolbar and connect signals/slots
    auto* toolbar = addToolBar("Main Toolbar");

    // Lock the toolbar position for a consistent UI layout
    toolbar->setMovable(false);

    // Create the Refresh action with MainWindow as parent
    auto* refreshAction = new QAction("Refresh", this);

    // Add the Refresh button to the toolbar
    toolbar->addAction(refreshAction);

    // When the user clicks the Refresh button
    // Invoke the slot to update metrics on DashboardWidget
    connect(refreshAction, &QAction::triggered,
            dashboard_, &DashboardWidget::onRefreshRequested);
}

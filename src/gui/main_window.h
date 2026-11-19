#pragma once

#include <QMainWindow>

// Forward declaration for the central widget
class DashboardWidget;

class MainWindow : public QMainWindow {
    // Required macro for any class using Qt's Signal/Slot mechanism
    Q_OBJECT

public:
    // Initialize the window, toolbar, and central widget
    explicit MainWindow(QWidget* parent = nullptr);

private:
    // Separate helper method to build the toolbar for cleaner code
    void setupToolbar();

    // Pointer holding the Dashboard widget for connecting signals/slots
    DashboardWidget* dashboard_ = nullptr;
};

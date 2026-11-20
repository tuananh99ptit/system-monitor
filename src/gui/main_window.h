#pragma once

#include <QMainWindow>

// Widget displaying metrics in the central area
class DashboardWidget;

// Top toolbar widget
class TopToolbar;

// Qt wrapper encapsulating UnixSocketClient and ClientData
class SocketClient;

class MainWindow : public QMainWindow {
    // Required macro for any class using Qt's Signal/Slot mechanism
    Q_OBJECT

public:
    // Initialize the UI and connect signals/slots
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    // Slot triggered when TopToolbar emits the refreshClicked() signal
    void onRefreshClicked();

private:
    // Toolbar containing the Refresh button
    TopToolbar* toolbar_ = nullptr;

    // Dashboard displaying system metrics
    DashboardWidget* dashboard_ = nullptr;

    // Wrapper communicating with the monitor service via Unix socket
    SocketClient* socketClient_ = nullptr;
};

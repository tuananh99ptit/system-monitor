#pragma once

#include <QWidget>
#include "monitor_data.h"

// Forward declaration to reduce header dependencies
class QLabel;

class DashboardWidget : public QWidget {
    Q_OBJECT // Required for classes using Qt's signal/slot or meta-object system

public:
    // Initialize the layout and initial labels
    explicit DashboardWidget(QWidget* parent = nullptr);

    // Update the text for all metric labels
    void updateMetrics(int cpuPercent, int memoryPercent, int diskPercent, int processCount);

public slots:
    // Slot triggered when the Refresh button on the toolbar is clicked
    void onRefreshRequested();

    // Update CPU, Memory, Disk, and Process metrics from SystemInfo
    void updateStatus(const SystemInfo& info);

private:
    // Label displaying CPU usage percentage
    QLabel* cpuLabel_ = nullptr;

    // Label displaying RAM usage percentage
    QLabel* memoryLabel_ = nullptr;

    // Label displaying Disk usage percentage
    QLabel* diskLabel_ = nullptr;

    // Label displaying total process count
    QLabel* processLabel_ = nullptr;

    // Count the number of refreshes to generate test data
    int refreshCount = 0;
};

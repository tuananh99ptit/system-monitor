#pragma once

#include <QWidget>

class QLabel;

class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget* parent = nullptr);

private:
    // Label displaying CPU usage percentage
    QLabel* cpuLabel_ = nullptr;

    // Label displaying RAM usage percentage
    QLabel* memoryLabel_ = nullptr;

    // Label displaying Disk usage percentage
    QLabel* diskLabel_ = nullptr;

    // Label displaying total process count
    QLabel* processLabel_ = nullptr;
};

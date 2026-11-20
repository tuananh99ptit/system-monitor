#include "dashboard_widget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QRandomGenerator>

DashboardWidget::DashboardWidget(QWidget* parent) : QWidget(parent) {
    // Initial mock CPU data
    cpuLabel_ = new QLabel("CPU : 0%", this);

    // Initial mock RAM data
    memoryLabel_ = new QLabel("Memory : 0%", this);

    // Initial mock Disk data
    diskLabel_ = new QLabel("Disk : 0%", this);

    // Mock process count data
    processLabel_ = new QLabel("Process : 0", this);

    // Vertical layout attached directly to DashboardWidget
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(cpuLabel_);
    layout->addWidget(memoryLabel_);
    layout->addWidget(diskLabel_);
    layout->addWidget(processLabel_);
    setLayout(layout);
}

void DashboardWidget::updateMetrics(int cpuPercent, int memoryPercent, int diskPercent, int processCount) {
    // Update CPU text
    cpuLabel_->setText(QString("CPU : %1%").arg(cpuPercent));

    // Update Memory text
    memoryLabel_->setText(QString("Memory : %1%").arg(memoryPercent));

    // Update Disk text
    diskLabel_->setText(QString("Disk : %1%").arg(diskPercent));

    // Update Process text
    processLabel_->setText(QString("Process : %1").arg(processCount));
}

void DashboardWidget::updateStatus(const SystemInfo& info) {
    // Push real metrics from SystemInfo to the UI
    updateMetrics(
        static_cast<int>(info.cpuUsage),                               // CPU from service/IPC
        static_cast<int>(info.memoryUsage),                            // Memory from service/IPC
        static_cast<int>(info.diskUsage),                              // Disk from service/IPC
        info.processCount                                              // Process count from service/IPC
    );
}

void DashboardWidget::onRefreshRequested() {
    ++refreshCount;
}

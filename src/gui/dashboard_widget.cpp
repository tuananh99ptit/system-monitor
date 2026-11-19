#include "dashboard_widget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QRandomGenerator>

DashboardWidget::DashboardWidget(QWidget* parent) : QWidget(parent) {
    // Initial mock CPU data
    cpuLabel_ = new QLabel("CPU : 25%", this);

    // Initial mock RAM data
    memoryLabel_ = new QLabel("Memory : 60%", this);

    // Initial mock Disk data
    diskLabel_ = new QLabel("Disk : 40%", this);

    // Mock process count data
    processLabel_ = new QLabel("Process : 243", this);

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

void DashboardWidget::onRefreshRequested() {
    // Qt6 global random number generator
    auto* rng = QRandomGenerator::global();

    // Random CPU value between 5% and 95%
    const int cpu = rng->bounded(5, 96);

    // Random Memory value between 20% and 94%
    const int mem = rng->bounded(20, 95);

    // Random Disk value between 10% and 89%
    const int disk = rng->bounded(10, 90);

    // Random process count between 120 and 449
    const int procs = rng->bounded(120, 450);

    // Push new metrics to the UI
    updateMetrics(cpu, mem, disk, procs);
}

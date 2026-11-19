#include "dashboard_widget.h"

#include <QLabel>
#include <QVBoxLayout>

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

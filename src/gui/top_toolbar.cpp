#include "top_toolbar.h"

#include <QHBoxLayout>
#include <QPushButton>

TopToolbar::TopToolbar(QWidget* parent) : QWidget(parent) {
    // Create the Refresh button with TopToolbar as parent
    refreshButton_ = new QPushButton("Refresh", this);

    // When the Refresh button is clicked -> Forward it as TopToolbar's refreshClicked() signal
    connect(refreshButton_, &QPushButton::clicked,     // When the Refresh button is clicked
            this, &TopToolbar::refreshClicked);
    
    // Horizontal layout attached directly to TopToolbar
    auto* layout = new QHBoxLayout(this);

    // Add the Refresh button to the horizontal layout
    // Push the button to the left for a clean UI layout
    // Apply the layout to TopToolbar
    layout->setContentsMargins(8, 6, 8, 6);
    layout->addWidget(refreshButton_);
    layout->addStretch();
    setLayout(layout);
}

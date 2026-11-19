#pragma once

#include <QWidget>

// Forward declaration for the Refresh button
class QPushButton;

class TopToolbar : public QWidget {
    // Enable Meta-Object system to declare signals and use connect()
    Q_OBJECT;

public:
    // Initialize the toolbar and the Refresh button
    explicit TopToolbar(QWidget* parent = nullptr);

signals:
    // Signal emitted when the user clicks the Refresh button
    void refreshClicked();

private:
    // Pointer holding the Refresh button on the toolbar
    QPushButton* refreshButton_ = nullptr;
};

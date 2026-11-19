#pragma once

#include <QMainWindow>

class MainWindow : public QMainWindow {
    // Required macro for any class using Qt's Signal/Slot mechanism
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
};

#pragma once

#include <QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT // Macro bắt buộc cho mọi class sử dụng hệ thống Signal/Slot của Qt

public:
    explicit MainWindow(QWidget* parent = nullptr);
};
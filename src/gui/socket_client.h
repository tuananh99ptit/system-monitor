#pragma once
#include "client_data.h"
#include "monitor_data.h"
#include "unix_socket_client.h"
#include <QObject>
#include <QString>

class SocketClient : public QObject {
    // Bật Meta-Object để dùng signals và kết nối slot
    Q_OBJECT;
public:
    // Khởi tạo wrapper IPC gắn với QObject parent
    explicit SocketClient(const QString& socketPath = QStringLiteral("/tmp/system_monitor.sock"), QObject* parent = nullptr);

    // Dừng luồng socket an toàn khi hủy wrapper
    ~SocketClient()override;

    // Khởi động kết nối / worker thread tới service
    bool connectToService();

    // Dừng kết nối và giải phóng worker thread
    void disconnectFromService();

    // Đọc trạng thái mới nhất từ ClientData và phát signal
    void requestStatus();

signals:
    // Phát dữ liệu SystemInfo sang cho DashboardWidget
    void statusReceived(const SystemInfo& info);

    // Thông báo trạng thái kết nối IPC tới UI
    void connectionStatusChanged(const QString& status);

private:
    // Lưu dữ liệu thread-safe nhận từ IPC layer
    ClientData clientData_;

    // Client Unix domain socket tái sử dụng từ Phase 4
    UnixSocketClient client_;
};
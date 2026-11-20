#include "socket_client.h"

// Khởi tạo UnixSocketClient với socket path và ClientData dùng chung
SocketClient::SocketClient(const QString& socketPath, QObject* parent)
    :QObject(parent), client_(clientData_, socketPath.toStdString()) {}

SocketClient::~SocketClient() {
    // Đảm bảo dừng luồng nền trước khi hủy đối tượng
    disconnectFromService();
}

bool SocketClient::connectToService() {
    if(client_.isRunning()) {
        // Báo trạng thái đang kết nối
        emit connectionStatusChanged(QStringLiteral("Connected to IPC service"));
        return true;
    }

    // Khởi chạy background thread kết nối Unix socket
    const bool started = client_.start();
    if(started) {
        // Trạng thái hoạt động
        emit connectionStatusChanged(QStringLiteral("IPC client started (/tmp/system_monitor.sock)"));
    } else {
        // Báo lỗi nếu không khởi chạy được
        emit connectionStatusChanged(QStringLiteral("Failed to start IPC client"));
    }
    return started;
}

void SocketClient::disconnectFromService() {
    // Chỉ gọi stop() khi luồng đang hoạt động
    if(client_.isRunning()) {
        // Dừng thread và đóng socket FD an toàn
        client_.stop();
        // Cập nhật trạng thái ngắt kết nối
        emit connectionStatusChanged(QStringLiteral("Disconnected from IPC Service"));
    }
}

void SocketClient::requestStatus(){
    // Chưa nhận frame dữ liệu nào từ server
    if(!clientData_.hasData()) {
        // Báo đang chờ dữ liệu
        emit connectionStatusChanged(QStringLiteral("Waiting for data from monitor service..."));
        return;
    }
    // Lấy bản sao dữ liệu thread-safe từ base MonitorData
    const SystemInfo info = clientData_.Get();
    // Phát signal để DashboardWidget cập nhật giao diện
    emit statusReceived(info);
}

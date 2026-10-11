#include "producer.h"
#include "consumer.h"
#include <chrono>
#include <string>
#include <thread>

int main() {
    ThreadSafeQueue<std::string> queue;

    Producer producer(queue);
    Consumer consumer(queue);

    producer.Start();
    consumer.Start();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    producer.Stop();    // 1. ngừng sản xuất
    queue.Close();      // 2. báo consumer: hết dữ liệu
    consumer.Join();    // 3. chờ consumer xử lý nốt rồi thoát
    return 0;
}
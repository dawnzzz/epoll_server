#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#include "tlvheader.h"

const int PORT = 8080;
const char* SERVER_IP = "127.0.0.1";
const int NUM_THREADS = 100;       // 并发线程数
const int REQUESTS_PER_THREAD = 100; // 每个线程的请求数
const int MESSAGE_SIZE = 64;    // 每次发送的消息大小

std::atomic<int> total_requests{0};
std::atomic<int> successful_requests{0};
std::atomic<long long> total_response_time{0};

// 生成指定长度的随机字母数字字符串
std::vector<char> generateRandomPackage(int length) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static int start = 0;
    TLVHeader header;
    header.type = htonl(TLVHEADER_TYPE::STRING);
    header.length = htonl(MESSAGE_SIZE);
    std::vector<char> packet(TLV_HEADER_LENGTH + MESSAGE_SIZE);
    memcpy(packet.data(), &header, TLV_HEADER_LENGTH);
    // memset(packet.data()+TLV_HEADER_LENGTH, 'A', MESSAGE_SIZE);
    std::vector<char> randomString;

    // 初始化随机数种子
    // std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (int i = 0; i < length; ++i) {
        // 随机选择一个字符添加到字符串中
        packet[TLV_HEADER_LENGTH+i] = alphanum[start++ % (sizeof(alphanum) - 1)];
    }

    return packet;
}

char hashVector(const std::vector<char>& input) {
    if (input.empty()) {
        return 0; // 如果向量为空，返回 0
    }

    char result = input[0];
    for (size_t i = 1; i < input.size(); ++i) {
        result &= input[i];
    }
    return result;
}


void client_thread(int thread_id) {
    
    // std::vector<char> packet(MESSAGE_SIZE);
    // memset(packet.data(), 'A', MESSAGE_SIZE);

    for (int i = 0; i < REQUESTS_PER_THREAD; ++i) {

        auto packet = generateRandomPackage(MESSAGE_SIZE);
        auto hash = hashVector(packet);
        // std::cout << packet.size() << std::endl;
        total_requests++;
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            std::cerr << "Thread " << thread_id << ": Failed to create socket: " << strerror(errno) << std::endl;
            continue;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
            std::cerr << "Thread " << thread_id << ": Invalid address/Address not supported: " << SERVER_IP << std::endl;
            close(sock);
            continue;
        }

        auto start_time = std::chrono::high_resolution_clock::now();

        if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Thread " << thread_id << ": Connection failed: " << strerror(errno) << std::endl;
            close(sock);
            continue;
        }

        // 发送数据
        if (write(sock, packet.data(), MESSAGE_SIZE+TLV_HEADER_LENGTH) != MESSAGE_SIZE+TLV_HEADER_LENGTH) {
            std::cerr << "Thread " << thread_id << ": Write failed: " << strerror(errno) << std::endl;
            close(sock);
            continue;
        }
        // std::cout << packet.size() << std::endl;

        // 接收响应
        std::vector<char> received(MESSAGE_SIZE+TLV_HEADER_LENGTH);
        int bytes_received = read(sock, packet.data(), MESSAGE_SIZE+TLV_HEADER_LENGTH);
        auto received_hash = hashVector(received);

        // 长度 & 哈希校验
        // std::cout << bytes_received << received_hash << hash << std::endl;
        if (bytes_received != MESSAGE_SIZE+TLV_HEADER_LENGTH || hash != received_hash) {
            std::cerr << "Thread " << thread_id << ": Read failed or incomplete: " << strerror(errno) << std::endl;
            close(sock);
            continue;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        long long response_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

        total_response_time += response_time;
        successful_requests++;
        

        close(sock);
    }
}

int main() {
    std::vector<std::thread> threads;
    auto start_time = std::chrono::high_resolution_clock::now();

    // 启动多个线程模拟并发客户端
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(client_thread, i);
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    long long total_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    // 输出统计结果
    std::cout << "Pressure test completed." << std::endl;
    std::cout << "Total requests: " << total_requests.load() << std::endl;
    std::cout << "Successful requests: " << successful_requests.load() << std::endl;
    std::cout << "Total time: " << total_time / 1000 << " ms" << std::endl;
    std::cout << "Average response time: " << total_response_time.load() / successful_requests.load() << " us" << std::endl;
    std::cout << "Requests per second: " << (successful_requests.load() * 1000000LL) / total_time << std::endl;

    return 0;
}
#ifndef BUFFERPOLL_H
#define BUFFERPOLL_H
#include <vector>
#include <mutex>

const int MIN_SIZE = 1024;
const int MAX_SIZE = 2048;

class BufferPool {
private:
    std::vector<std::vector<char>> pool;
    std::mutex mtx;
    size_t BUFFER_SIZE;
    size_t BUFFER_POOL_SIZE;
public:
    BufferPool(size_t chunk_size, size_t pool_size) {
        for(size_t i = 0; i < pool_size; ++i) {
            std::vector<char> buffer;
            buffer.reserve(MAX_SIZE);
            pool.push_back(buffer);
        }

        BUFFER_SIZE = chunk_size;
        BUFFER_POOL_SIZE = pool_size;
    }

    std::vector<char> get_buffer() {
        std::lock_guard<std::mutex> lock(mtx);
        if(pool.empty()) {
            std::vector<char> buffer(MIN_SIZE);
            return buffer;
        }
        auto buf = std::move(pool.back());
        pool.pop_back();
        buf.resize(MIN_SIZE);
        return buf;
    }

    void return_buffer(std::vector<char>&& buf) {
        if (buf.capacity() < MIN_SIZE || buf.capacity() > MAX_SIZE) {
            return;
        }
        std::lock_guard<std::mutex> lock(mtx);
        if(pool.size() < BUFFER_POOL_SIZE) {
            buf.clear();
            // buf.resize(BUFFER_SIZE);
            pool.push_back(std::move(buf));
        }
    }
};

static BufferPool BUFFER_POOL(MIN_SIZE, 10000);

#endif
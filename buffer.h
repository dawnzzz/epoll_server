#pragma once

#include <utility>
#include <vector>
#include <memory.h>
#include <string>

#include "bufferpool.h"

class Buffer{
    public:
        Buffer() = default;
        ~Buffer() {
            BUFFER_POOL.return_buffer(std::move(buf_));
        }

        std::vector<char> buf() const;

        void set_buf(std::vector<char>&& buf);

        size_t Size() const;
        void Append(std::vector<char>, int size);
        void Clear();
    
    private:
        std::vector<char> buf_ = BUFFER_POOL.get_buffer();
};
#pragma once

#include <memory.h>
#include <string>

class Buffer{
    public:
        Buffer() = default;
        ~Buffer() = default;

        const std::string &buf() const;
        const char *c_str() const;

        void set_buf(const char *buf, size_t size);

        size_t Size() const;
        void Append(const char *_str, int _size);
        void Clear();
    
    private:
        std::string buf_;
};
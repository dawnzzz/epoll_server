#pragma once

#include <memory>
#include <vector>
#include <string>
#include <cstring>

static const int DEFAULT_INIT_SIZE = 256; // 初始化开辟空间长度

class Buffer{
    public:

        Buffer();
        ~Buffer();

        char *begin();
        const char *begin() const;

        char *beginread();
        const char *beginread() const;

        char *beginwrite();
        const char *beginwrite() const;


        void Append(const char *message, int len);
        void Append(const std::string &message);


        int readablebytes() const;
        int writablebytes() const;

        char *Peek();
        const char *Peek() const;


        void Retrieve(int len);
        std::string RetrieveAsString(int len);

        void RetrieveAll();

        void EnsureWritableBytes(int len);

    private:
        std::vector<char> buffer_;
        int read_index_;
        int write_index_;
};
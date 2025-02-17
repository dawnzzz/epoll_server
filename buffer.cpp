#include "buffer.h"

std::vector<char> Buffer::buf() const { return buf_; }

void Buffer::set_buf(std::vector<char>&& buf) {
  buf_.swap(buf);
}

size_t Buffer::Size() const { return buf_.size(); }

void Buffer::Append(std::vector<char> str, int size) {
  for (int i = 0; i < size; ++i) {
    buf_.push_back(str[i]);
  }
}

void Buffer::Clear() { buf_.clear(); }
#include <string>
#include <cstring>

#include "buffer.h"

Buffer::Buffer() :
    buffer_(DEFAULT_INIT_SIZE),
    read_index_(0),
    write_index_(0){}

Buffer::~Buffer(){}

char *Buffer::begin() { return &*buffer_.begin(); }
const char *Buffer::begin() const { return &*buffer_.begin(); }
char* Buffer::beginread() { return begin() + read_index_; } 
const char* Buffer::beginread() const { return begin() + read_index_; }
char* Buffer::beginwrite() { return begin() + write_index_; }
const char* Buffer::beginwrite() const { return begin() + write_index_; }


void Buffer::Append(const char* message, int len) {
    EnsureWritableBytes(len);
    std::copy(message, message + len, beginwrite());
    write_index_ += len;
}


int Buffer::readablebytes() const { return write_index_ - read_index_; }
int Buffer::writablebytes() const { return static_cast<int>(buffer_.size()) - write_index_; } 

char *Buffer::Peek() { return beginread(); }
const char *Buffer::Peek() const { return beginread(); }

void Buffer::Retrieve(int len){
    if(len + read_index_ < write_index_){
        read_index_ += len;
    }else{
        RetrieveAll();
    }
}

void Buffer::RetrieveAll(){
    write_index_ = 0;
    read_index_ = write_index_;
}

void Buffer::EnsureWritableBytes(int len){
    if(writablebytes() >= len)
        return;
    
    buffer_.resize(write_index_ + len);
}
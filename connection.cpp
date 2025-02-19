#include "connection.h"
#include "buffer.h"
#include "channel.h"
#include "eventloop.h"
#include <cerrno>
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>

#include "logger.h"


Connection::Connection(EventLoop *loop, int connfd): client_fd(connfd), event_loop(loop){

    if (loop != nullptr)
    {
        // 创建channel
        channel = std::make_unique<Channel>(connfd, loop);
        channel->EnableET();   // 边缘触发
        channel->SetReadCallBack(std::bind(&Connection::HandleMessage, this));
        channel->SetWriteCallBack(std::bind(&Connection::HandleWrite, this));
    }
    read_buffer = std::make_unique<std::vector<char>>();
    send_buffer = std::make_unique<Buffer>();
    
    tlv_header = std::make_unique<TLVHeader>();
    // tlv_header_buffer = std::make_unique<std::vector<char>>(TLV_HEADER_LENGTH);
    read_tlv_header_length = 0;
    read_body_length = 0;
    ready_to_handle_message = false;
}

Connection::~Connection(){
    close(client_fd);
}

void Connection::ConnectionEstablished(){
    // Logger.Debug("Connection::ConnectionEstablished", "", "ConnectionEstablished clientFd: "+ std::to_string(client_fd));
    state = ConnectionState::Connected;
    channel->Tie(shared_from_this());
    channel->EnableRead();
    if (on_connect_){
        on_connect_(shared_from_this());
    }
}

void Connection::ConnectionDestructor(){
    event_loop->DeleteChannel(channel.get());
}

void Connection::SetConnectionCallBack(std::function<void(const std::shared_ptr<Connection> &)> const &fn){
    on_connect_ = std::move(fn);
}
void Connection::SetCloseCallback(std::function<void(const std::shared_ptr<Connection> &)> const  &fn) { 
    on_close_callback = std::move(fn); 
}
void Connection::SetMessageCallback(std::function<void(const std::shared_ptr<Connection> &)> const &fn) { 
    on_message_callback = std::move(fn);
}


void Connection::HandleClose() {
    // Logger.Debug("Connection::HandleClose", "", "clientFd = " + std::to_string(client_fd));
    if (state != ConnectionState::Disconected)
    {
        state = ConnectionState::Disconected;
        if(on_close_callback){
            on_close_callback(shared_from_this());
        }
    }
}

void Connection::HandleMessage(){
    
    Read();
    // Logger.Debug("Connection::HandleMessage", "", "read_buffer size = " + std::to_string(read_buffer->Size()));
    if (on_message_callback && ready_to_handle_message)
    {
        on_message_callback(shared_from_this());
        ready_to_handle_message = false;
        read_tlv_header_length = 0;
        read_body_length = 0;
    }
}

void Connection::HandleWrite(){

    WriteNonBlocking();
}

EventLoop *Connection::Loop() { return event_loop; }
int Connection::ClientFd() { return client_fd; }
Connection::ConnectionState Connection::State() { return state; }
const std::vector<char> *Connection::ReadBuffer(){ return read_buffer.get(); }
Buffer *Connection::SendBuffer() { return send_buffer.get(); }

void Connection::Send(const char *msg, size_t len){
    // Logger.Info("Connection::Send", "", "msg size = " + std::to_string(len) +" send_buffer size = " + std::to_string(send_buffer->readablebytes()));

    
    int send_size = 0;

    // 添加tlv头部
    TLVHeader header;
    header.type = htonl(TLVHEADER_TYPE::STRING);
    header.length = htonl(len);
    int response_len = TLV_HEADER_LENGTH + len;
    char response[response_len];
    // ** 这里的拷贝是必须的，因为业务层不感知tlv header **
    memcpy(response, &header, TLV_HEADER_LENGTH);
    header.type = ntohl(TLVHEADER_TYPE::STRING);
    header.length = ntohl(len);
    
    memcpy(response+TLV_HEADER_LENGTH, msg, len);
    int remaining = response_len;

    if (send_buffer->readablebytes() == 0){
        send_size = write(client_fd, response, response_len);
        // Logger.Info("Connection::Send", "", "send_size = " + std::to_string(send_size));
        if(send_size >= 0){
            // 说明发送了部分数据
            remaining -= send_size;
        } else if((send_size == -1) && 
                    ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR))){
            Logger.Error("Connection::Send", strerror(errno), "send again");
            send_size = 0;
        } else{
            Logger.Error("Connection::Send", strerror(errno), "send failed");
            HandleClose();
            return;
        }
    }

    // Logger.Info("Connection::Send", "", "remaining = " + std::to_string(remaining));
    // 剩余数据放入send buffer
    if(remaining > 0){
        send_buffer->Append(response + send_size, remaining);

        // 注册写事件
        channel->EnableWrite();
    }
}

void Connection::Send2(const char *msg, size_t len){
    // Logger.Info("Connection::Send", "", "msg size = " + std::to_string(len) +" send_buffer size = " + std::to_string(send_buffer->readablebytes()));

    
    int send_size = 0;

    // 添加tlv头部
    TLVHeader header;
    header.type = htonl(TLVHEADER_TYPE::STRING);
    header.length = htonl(len);    

    send_buffer->Append((char *)&header, TLV_HEADER_LENGTH);
    send_buffer->Append(msg, len);

    // 注册写事件
    channel->EnableWrite();
}

void Connection::SendWithTwoSysCall(const char *msg, size_t len){
    Logger.Info("Connection::SendWithTwoSysCall", "", "msg size = " + std::to_string(len) +" send_buffer size = " + std::to_string(send_buffer->readablebytes()));

    // 添加tlv头部
    TLVHeader header;
    header.type = htonl(TLVHEADER_TYPE::STRING);
    header.length = htonl(len);

    Logger.Info("Connection::SendWithTwoSysCall", "", "header.length = " + std::to_string(ntohl(header.length)));
    
    SendInternal((char*)&header, TLV_HEADER_LENGTH);
    SendInternal(msg, len);
}


void Connection::SendInternal(const char *msg, size_t len){
    Logger.Info("Connection::SendInternal", "", "msg size = " + std::to_string(len) +" send_buffer size = " + std::to_string(send_buffer->readablebytes()));

    
    int send_size = 0;
    int response_len = len;
    int remaining = response_len;

    if (send_buffer->readablebytes() == 0){
        send_size = write(client_fd, msg, response_len);
        Logger.Info("Connection::SendInternal", "", "send_size = " + std::to_string(send_size));
        if(send_size >= 0){
            // 说明发送了部分数据
            remaining -= send_size;
        } else if((send_size == -1) && 
                    ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR))){
            Logger.Error("Connection::SendInternal", strerror(errno), "send again");
            send_size = 0;
        } else{
            Logger.Error("Connection::SendInternal", strerror(errno), "send failed");
            HandleClose();
            return;
        }
    }

    // Logger.Info("Connection::Send", "", "remaining = " + std::to_string(remaining));
    // 剩余数据放入send buffer
    if(remaining > 0){
        send_buffer->Append(msg + send_size, remaining);

        // 注册写事件
        channel->EnableWrite();
    }
}

void Connection::Read()
{
    ReadNonBlocking();
}

void Connection::Write(){
    WriteNonBlocking();
}


void Connection::ReadNonBlocking(){
    // std::vector<char> buf = BUFFER_POOL.get_buffer();
    while(true){
        if (read_tlv_header_length < TLV_HEADER_LENGTH)
        {
            // 读取tlv头部
            ssize_t bytes_read = read(client_fd, reinterpret_cast<unsigned char*>(tlv_header.get())+read_tlv_header_length, TLV_HEADER_LENGTH-read_tlv_header_length);
            // Logger.Info("Connection::ReadNonBlocking", "", "read tlv header length "+ std::to_string(bytes_read));
            if (bytes_read > 0) {
                read_tlv_header_length += bytes_read;
                
                if (read_tlv_header_length == TLV_HEADER_LENGTH) {
                    // 读取到了完整的tlv头部
                    // memcpy(&tlv_header, tlv_header_buffer->data(), TLV_HEADER_LENGTH);
                    tlv_header->type = htonl(tlv_header->type);
                    tlv_header->length = htonl(tlv_header->length);
                    if (tlv_header->length > MAX_TLV_LENGTH) {
                        // tlv头部长度过长
                        Logger.Error("Connection::ReadNonBlocking", "", "read tlv header length too long "+ std::to_string(tlv_header->length));
                        HandleClose();
                        break;
                    }
                    
                    // 预分配body空间
                    read_buffer->resize(tlv_header->length);
                    // Logger.Info("Connection::ReadNonBlocking", "", "read tlv header finished, body length "+ std::to_string(tlv_header->length));
                }
            }else if(bytes_read == -1 && errno == EINTR){
                continue;
            }else if((bytes_read == -1) && (
                (errno == EAGAIN) || (errno == EWOULDBLOCK))){
                break;
            }else if (bytes_read == 0){
                HandleClose();
                break;
            }else{
                HandleClose();
                break;
            }
        } else {
            // Logger.Info("Connection::ReadNonBlocking", "", "read_body_length "+ std::to_string(read_body_length));
            ssize_t bytes_read = read(client_fd, read_buffer->data()+read_body_length, read_buffer->size()-read_body_length);
            // Logger.Info("Connection::ReadNonBlocking", "", "read length "+ std::to_string(bytes_read));
            if(bytes_read > 0){
                read_body_length += bytes_read;
                if (read_body_length == tlv_header->length) {
                    ready_to_handle_message = true;
                    break;
                }
                // Logger.Info("Connection::ReadNonBlocking", "", "append, read_buffer size = "+ std::to_string(read_buffer->size()));
            }else if(bytes_read == -1 && errno == EINTR){
                Logger.Info("Connection::ReadNonBlocking", strerror(errno), "read intr");
                continue;
            }else if((bytes_read == -1) && (
                (errno == EAGAIN) || (errno == EWOULDBLOCK))){
                Logger.Info("Connection::ReadNonBlocking", strerror(errno), "read again");
                break;
            }else if (bytes_read == 0){//
                Logger.Debug("Connection::ReadNonBlocking", strerror(errno), "read bytes == 0");
                if (read_body_length == tlv_header->length) {
                    ready_to_handle_message = true;
                    break;
                }
                HandleClose();
                break;
            }else{
                Logger.Error("Connection::ReadNonBlocking", strerror(errno), "read failed");
                HandleClose();
                break;
            }

        }
    }
}

void Connection::WriteNonBlocking(){
    
    int remaining = send_buffer->readablebytes();
    int send_size = write(client_fd, send_buffer->Peek(), remaining);
    if((send_size == -1) && 
                ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR))){
        Logger.Error("Connection::WriteNonBlocking", strerror(errno), "send failed");
        send_size = 0;
        channel->EnableWrite();
    } else if (send_size == -1){
        Logger.Error("Connection::WriteNonBlocking", strerror(errno), "send failed");
        HandleClose();
        return;
    }

    remaining -= send_size;
    send_buffer->Retrieve(send_size);
}
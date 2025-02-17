#include "connection.h"
#include "buffer.h"
#include "channel.h"
#include "eventloop.h"
#include <cerrno>
#include <memory>
#include <string>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>

#include "logger.h"


Connection::Connection(EventLoop *loop, int connfd): client_fd(connfd), event_loop(loop){

    if (loop != nullptr)
    {
        // 创建channel
        channel = std::make_unique<Channel>(connfd, loop);
        channel->EnableET();   // 边缘触发
        channel->SetReadCallBack(std::bind(&Connection::HandleMessage, this));
    }
    read_buffer = std::make_unique<Buffer>();
    send_buffer = std::make_unique<Buffer>();
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
    if (on_message_callback)
    {
        on_message_callback(shared_from_this());
    }
}

EventLoop *Connection::Loop() { return event_loop; }
int Connection::ClientFd() { return client_fd; }
Connection::ConnectionState Connection::State() { return state; }
void Connection::SetSendBuffer(std::vector<char>&& str) { send_buffer->set_buf(std::move(str)); }
Buffer *Connection::ReadBuffer(){ return read_buffer.get(); }
Buffer *Connection::SendBuffer() { return send_buffer.get(); }

void Connection::Send(std::vector<char>&& msg){
    SetSendBuffer(std::move(msg));
    // Logger.Debug("Connection::Send", "", "msg size = " + std::to_string(msg.size()) +" send_buffer size = " + std::to_string(send_buffer->Size()));
    Write();
}

void Connection::Read()
{
    read_buffer->Clear();
    ReadNonBlocking();
}

void Connection::Write(){
    WriteNonBlocking();
    send_buffer->Clear();
}


void Connection::ReadNonBlocking(){
    std::vector<char> buf = BUFFER_POOL.get_buffer();
    while(true){
        ssize_t bytes_read = read(client_fd, buf.data(), buf.size());
        // Logger.Info("Connection::ReadNonBlocking", "", "read length "+ std::to_string(bytes_read));
        if(bytes_read > 0){
            read_buffer->Append(buf, bytes_read);
            // Logger.Info("Connection::ReadNonBlocking", "", "append, read_buffer size = "+ std::to_string(read_buffer->Size()));
        }else if(bytes_read == -1 && errno == EINTR){
            continue;
        }else if((bytes_read == -1) && (
            (errno == EAGAIN) || (errno == EWOULDBLOCK))){
            break;
        }else if (bytes_read == 0){//
            HandleClose();
            break;
        }else{
            HandleClose();
            break;
        }
    }
}

void Connection::WriteNonBlocking(){
    
    std::vector<char> buf(send_buffer->buf());
    int data_size = send_buffer->Size();
    int data_left = data_size;

    while(data_left > 0){
        
        ssize_t bytes_write = write(client_fd, buf.data() + data_size - data_left, data_left);
        // Logger.Debug("Connection::WriteNonBlocking", "", "write length "+ std::to_string(bytes_write));
        if(bytes_write == -1 && errno == EINTR){
            continue;
        }
        if(bytes_write == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
            break;
        }
        if(bytes_write == -1){
            HandleClose();
            break;
        }
        data_left -= bytes_write;
    }
}
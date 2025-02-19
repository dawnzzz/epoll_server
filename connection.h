#pragma once
#include <functional>
#include <memory>
#include <vector>

#include "eventloop.h"
#include "tlvheader.h"


class Buffer;
class Connection : public std::enable_shared_from_this<Connection>
{
public:
    enum ConnectionState
    {
        Invalid = 1,
        Connected,
        Disconected
    };
    
    Connection(EventLoop *loop, int connfd);
    ~Connection();

    // 初始化
    void ConnectionEstablished();

    // 销毁TcpConection
    void ConnectionDestructor();

    // 建立连接时调用回调函数
    void SetConnectionCallBack(std::function<void(const std::shared_ptr<Connection> &)> const &fn);
     // 关闭时的回调函数
    void SetCloseCallback(std::function<void(const std::shared_ptr<Connection> &)> const &fn);   
    // 接受到信息的回调函数                                  
    void SetMessageCallback(std::function<void(const std::shared_ptr<Connection> &)> const &fn); 

    // 设定send buf
    const std::vector<char> *ReadBuffer();
    Buffer *SendBuffer();


    void Send(const char *msg, size_t len);
    void Send2(const char *msg, size_t len);
    void SendWithTwoSysCall(const char *msg, size_t len);


    void HandleMessage(); // 当接收到信息时，进行回调

    void HandleWrite();

    // 当TcpConnection发起关闭请求时，进行回调，释放相应的socket.
    void HandleClose(); 


    ConnectionState State();
    EventLoop *Loop();
    int ClientFd();

    void SendInternal(const char *msg, size_t len);

private:
    void Read(); // 读操作
    void Write(); // 写操作


private:
    // 该连接绑定的Socket
    int client_fd;

    // 连接状态
    ConnectionState state;

    EventLoop *event_loop;

    std::unique_ptr<Channel> channel;
    std::unique_ptr<std::vector<char>> read_buffer;
    std::unique_ptr<Buffer> send_buffer;

    int read_tlv_header_length;
    int read_body_length;
    // std::unique_ptr<std::vector<char>> tlv_header_buffer;
    std::unique_ptr<TLVHeader> tlv_header;
    bool ready_to_handle_message;

    std::function<void(const std::shared_ptr<Connection> &)> on_close_callback;
    std::function<void(const std::shared_ptr<Connection> &)> on_message_callback;
    std::function<void(const std::shared_ptr<Connection> &)> on_connect_;

    void ReadNonBlocking();
    void WriteNonBlocking();

};
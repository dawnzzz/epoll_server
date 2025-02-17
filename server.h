#pragma once

#include <memory>
#include <functional>
#include <map>

class EventLoop;
class Connection;
class Acceptor;
class EventLoopThreadPool;
class InetAddress;
class Server {
public:
    Server(EventLoop *loop, const char *ip, const int port);
    ~Server();

    void Start();

    void setConnectionCallback(std::function < void(const std::shared_ptr<Connection> &)> const &fn);
    void SetMessageCallback(std::function < void(const std::shared_ptr<Connection> &)> const &fn);

    inline void HandleClose(const std::shared_ptr<Connection> &);
    // 进行一层额外的封装，以保证erase操作是由`main_reactor_`来操作的。
    inline void HandleCloseInLoop(const std::shared_ptr<Connection> &);

    // 接收到消息做的操作。
    inline void HandleNewConnection(int fd);
    
    // 定义线程数量
    void SetThreadNums(int thread_nums);

private:
    EventLoop *main_reactor;
    int next_conn_id_;

    std::unique_ptr<EventLoopThreadPool> thread_pool;

    std::unique_ptr<Acceptor> acceptor;
    std::map<int, std::shared_ptr<Connection>> connectionsMap;

    std::function<void(const std::shared_ptr<Connection> &)> on_connect;
    std::function<void(const std::shared_ptr<Connection> &)> on_message;

};

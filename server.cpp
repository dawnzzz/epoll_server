#include <memory>
#include <assert.h>
#include <iostream>
#include <string>

#include "server.h"
#include "connection.h"
#include "eventloop.h"
#include "acceptor.h"
#include "eventloopthreadpool.h"
#include "logger.h"

Server::Server(EventLoop *loop, const char * ip, const int port): main_reactor(loop), next_conn_id_(1){
    // 创建main reactor
    acceptor = std::make_unique<Acceptor>(main_reactor, ip, port);
    std::function<void(int)> cb = std::bind(&Server::HandleNewConnection, this, std::placeholders::_1);
    acceptor->SetNewConnectionCallBack(cb);

    // 创建线程池
    thread_pool = std::make_unique<EventLoopThreadPool>(loop);
}

Server::~Server(){
};

void Server::Start(){
    Logger.Info("Server::Start", "", "server starting");
    thread_pool->Start();

    Logger.Info("Server::Start", "", "server start succ, main_reactor running");
    main_reactor->Start();
}

inline void Server::HandleNewConnection(int fd){
    assert(fd != -1);
    
    // Logger.Info("Server::HandleNewConnection", "", "ClientFd: " + std::to_string(fd));
    // 获取一个sub reactor
    EventLoop *sub_reactor = thread_pool->NextEventLoop();
    // 建立连接
    std::shared_ptr<Connection> conn = std::make_shared<Connection>(sub_reactor,  fd);
    
    std::function<void(const std::shared_ptr<Connection> &)> cb = std::bind(&Server::HandleClose, this, std::placeholders::_1);
    conn->SetConnectionCallBack(on_connect);

    conn->SetCloseCallback(cb);
    conn->SetMessageCallback(on_message);
    connectionsMap[fd] = conn;

    conn->ConnectionEstablished();
}


inline void Server::HandleClose(const std::shared_ptr<Connection> & conn){
    // Logger.Info("Server:HandleClose", "", (std::string("current thread id: ") + std::to_string(CurrentThread::tid())).c_str());

    main_reactor->RunOneFunc(std::bind(&Server::HandleCloseInLoop, this, conn));
}

inline void Server::HandleCloseInLoop(const std::shared_ptr<Connection> & conn){
    auto it = connectionsMap.find(conn->ClientFd());
    assert(it != connectionsMap.end());
    connectionsMap.erase(connectionsMap.find(conn->ClientFd()));

    EventLoop *loop = conn->Loop();
    loop->QueueOneFunc(std::bind(&Connection::ConnectionDestructor, conn));
}

void Server::setConnectionCallback(std::function<void(const std::shared_ptr<Connection> &)> const &fn) { on_connect = std::move(fn); };
void Server::SetMessageCallback(std::function<void(const std::shared_ptr<Connection> &)> const &fn) { on_message = std::move(fn); };

void Server::SetThreadNums(int thread_nums) { thread_pool->SetThreadNums(thread_nums); }

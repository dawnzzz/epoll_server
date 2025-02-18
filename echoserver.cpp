
#include <string>
#include <thread>
#include <iostream>
#include <functional>
#include <arpa/inet.h>

#include "eventloop.h"
#include "connection.h"
#include "server.h"
#include "buffer.h"
#include "currentthread.h"
#include "config.h"
#include "logger.h"

class EchoServer{
    public:
        EchoServer(EventLoop *loop, const char *ip, const int port);
        ~EchoServer();

        void start();
        void onConnection(const std::shared_ptr<Connection> & conn);
        void onMessage(const std::shared_ptr<Connection> & conn);

        void SetThreadNums(int thread_nums);

    private:
        Server server_;
};

EchoServer::EchoServer(EventLoop *loop, const char *ip, const int port) :  server_(loop, ip, port){
    server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
    server_.SetMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1));
};
EchoServer::~EchoServer(){};

void EchoServer::start(){
    server_.Start();
}

void EchoServer::onConnection(const std::shared_ptr<Connection> & conn){
    // 获取接收连接的Ip地址和port端口
    int clnt_fd = conn->ClientFd();
    struct sockaddr_in peeraddr;
    socklen_t peer_addrlength = sizeof(peeraddr);
    getpeername(clnt_fd, (struct sockaddr *)&peeraddr, &peer_addrlength);

    // std::ostringstream oss;
    // oss << CurrentThread::tid()
    //           << " EchoServer::OnNewConnection : new connection from " << inet_ntoa(peeraddr.sin_addr) << ":" << ntohs(peeraddr.sin_port);
    // Logger.Debug("EchoServer::onConnection", "", oss.str());
};

void EchoServer::onMessage(const std::shared_ptr<Connection> & conn){
    // Logger.Info("EchoServer::onMessage", "", "clientFd = " + std::to_string(conn->ClientFd()));
    // Logger.Info("EchoServer::onMessage", "", "state = " + std::to_string(conn->State()));
    if (conn->State() == Connection::ConnectionState::Connected)
    {
        std::ostringstream oss;
        // oss << CurrentThread::tid() << " Message from clent, length = " << conn->ReadBuffer()->Size();
        // Logger.Info("EchoServer::onMessage", "", oss.str());
        conn->Send(conn->ReadBuffer()->c_str(), conn->ReadBuffer()->Size());
        conn->HandleClose();
    }
}

void EchoServer::SetThreadNums(int thread_nums) { server_.SetThreadNums(thread_nums); }

int main(int argc, char *argv[]){
    Config config;
    config.parse_arg(argc, argv);

    if (config.IS_ASYNC_LOGGER )
        Logger.StartAasync(config.LOG_FILE);

    std::ostringstream oss;
    oss << "ADDR = " << config.ADDR << " PORT = " << config.PORT << " THREAD_NUM = " << config.THREAD_NUM << " IS_ASYNC_LOGGER = " << config.IS_ASYNC_LOGGER << " LOG_FILE = " << config.LOG_FILE;
    Logger.Info("main", "", oss.str());
    
    int size = config.THREAD_NUM;
    EventLoop *loop = new EventLoop();
    EchoServer *server = new EchoServer(loop, config.ADDR, config.PORT);

    server->SetThreadNums(size);
    server->start();
    std::cout << "end" << std::endl;
    return 0;
}
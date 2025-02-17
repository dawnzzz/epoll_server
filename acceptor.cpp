#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <memory>

#include "logger.h"
#include "acceptor.h"

Acceptor::Acceptor(EventLoop *loop, const char * ip, const int port) :event_loop(loop), listened_fd(-1){
    Create();
    Bind(ip, port);
    Listen();
    // acceptor的channel，专门负责接收新的tcp连接
    accept_channel = std::make_unique<Channel>(listened_fd, loop);
    std::function<void()> cb = std::bind(&Acceptor::Accept, this);
    accept_channel->SetReadCallBack(cb);
    accept_channel->EnableRead();
}


Acceptor::~Acceptor(){
    event_loop->DeleteChannel(accept_channel.get());
    close(listened_fd);
};

void Acceptor::Create(){
    assert(listened_fd == -1);
    listened_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(listened_fd == -1){
        Logger.Fatal("Acceptor::Create", strerror(errno), "create socket failed");
    }

    int opt = 1;
    if (setsockopt(listened_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        // 复用地址
        Logger.Fatal("Acceptor::Create", strerror(errno), "setsockopt SO_REUSEADDR failed");
    }

    Logger.Error("Acceptor::Create", "", "create socket succ");
}

void Acceptor::Bind(const char *ip, const int port){
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if(bind(listened_fd, (struct sockaddr *)&addr, sizeof(addr))==-1){
        Logger.Error("Acceptor::Bind", strerror(errno), "bind failed");
    }

    Logger.Error("Acceptor::Bind", "", "bind socket succ");
}

void Acceptor::Listen(){
    assert(listened_fd != -1);
    if(listen(listened_fd, SOMAXCONN) == -1){
        Logger.Error("Acceptor::Listen", strerror(errno), "listen failed");
    }

    Logger.Error("Acceptor::Listen", "", "listen socket succ");
}

void Acceptor::Accept(){
    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    assert(listened_fd != -1);
    int clnt_fd = accept4(listened_fd, (struct sockaddr *)&client, &client_addrlength, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (clnt_fd == -1){
        // 错误，直接关闭客户端连接
        Logger.Error("Acceptor::Accept", strerror(errno), "accept failed");
        close(clnt_fd);
        return;
    }
    if(new_conntection_callback){
        new_conntection_callback(clnt_fd);
    }
}

void Acceptor::SetNewConnectionCallBack(std::function<void(int)> &callback){
    new_conntection_callback = std::move(callback);
}

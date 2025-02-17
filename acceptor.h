#pragma once

#include "eventloop.h"
#include <functional>
class Acceptor {
public:

    Acceptor(EventLoop *loop, const char * ip, const int port);
    ~Acceptor();

    void SetNewConnectionCallBack(std::function<void(int)>& callback);

    void Create();

    void Bind(const char* ip, int port);

    void Listen();

    void Accept();
private:

    EventLoop *event_loop;
    int listened_fd;
    std::unique_ptr<Channel> accept_channel;
    std::function<void(int)> new_conntection_callback;
};
#pragma once

#include <vector>
#include <sys/epoll.h>

class Channel;


class Epoller {
private:

public:
    Epoller();
    ~Epoller();

    void UndateChannel(Channel* chan);
    void DeleteChannel(Channel *chan);
    
    std::vector<Channel*> Epoll();

private:
    int epoll_fd;
    int event_num;
    struct epoll_event* epoll_events;
};
#include <sys/epoll.h>

#include "channel.h"
#include "eventloop.h"

Channel::Channel(int fd, EventLoop *loop) : client_fd(fd), event_loop(loop),
                                            listening_events(0), ready_events(0),
                                            in_epoll(false){};

Channel::~Channel() {}



void Channel::HandleEvent(){
    if(tied){
        std::shared_ptr<void> guard = tie.lock();
        HandleEventWithGuard();
    }else{
        HandleEventWithGuard();
    }
}

void Channel::Tie(const std::shared_ptr<void> &ptr){
    tied = true;
    tie = ptr;
}

void Channel::HandleEventWithGuard(){
    if (ready_events  & (EPOLLERR | EPOLLRDHUP | EPOLLHUP) ) {
        // 客户端关闭了连接

    }
    if (ready_events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (read_callback) {
            read_callback();
        }
    } 

    if (ready_events & EPOLLOUT) {
        if (write_callback) {
            write_callback();
        }
    }
}

void Channel::EnableET(){
    listening_events |= (EPOLLET);
    event_loop->UpdateChannel(this);
}

void Channel::EnableRead(){
    listening_events |= (EPOLLIN | EPOLLPRI);
    event_loop->UpdateChannel(this);
}

void Channel::EnableWrite(){
    listening_events |= (EPOLLOUT);
    event_loop->UpdateChannel(this);
}

int Channel::ClientFd() { return client_fd; }

int Channel::ListeningEvents() { return listening_events; }
int Channel::ReadyEvents() { return ready_events; }

bool Channel::IsInEpoll() { return in_epoll; }
void Channel::SetInEpoll(bool in_epoll_) { in_epoll = in_epoll_; }

void Channel::SetReadyEvents(int events){
    ready_events = events;
}

void Channel::SetReadCallBack(std::function<void()> const &callback) { read_callback = std::move(callback); }
void Channel::SetWriteCallBack(std::function<void()> const &callback) { write_callback = std::move(callback); }

bool Channel::IsFailed() { return failed;}
void Channel::SetFailed() { failed = true; }
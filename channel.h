#pragma once

#include <functional>
#include <memory>


class EventLoop;
class Channel {
private:
    int client_fd;

    EventLoop* event_loop;

    int listening_events;
    int ready_events;

    bool in_epoll = false;
    bool failed = false;


    std::function<void()> read_callback;
    std::function<void()> write_callback;

    bool tied;
    std::weak_ptr<void> tie;

public:
    
    Channel(int fd, EventLoop *loop);
    ~Channel();

    int ClientFd();
    bool IsInEpoll();
    void SetInEpoll(bool in_epoll=true);
    bool IsFailed();
    void SetFailed();
    int ListeningEvents();
    int ReadyEvents();
    void SetReadyEvents(int events);
    void SetReadCallBack(std::function<void()> const & cb);
    void SetWriteCallBack(std::function<void()> const & cb);

    void Tie(const std::shared_ptr<void> &ptr);
    void HandleEvent();
    void HandleEventWithGuard();
    void EnableET();
    void EnableRead(); 
    void EnableWrite(); 
};
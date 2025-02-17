#include <cerrno>
#include <cstring>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

#include "epoller.h"
#include "channel.h"
#include "logger.h"

#define EVENT_NUM 1024

Epoller::Epoller() {
    epoll_fd = epoll_create1(0);
    event_num = EVENT_NUM;
    epoll_events = new epoll_event[event_num];
}

Epoller::~Epoller() {
    if (epoll_fd != -1) {
        close(epoll_fd);
    }

    delete [] epoll_events;
}

std::vector<Channel*> Epoller::Epoll() {

    std::vector<Channel*> ready_channels;

    int n  = epoll_wait(epoll_fd, epoll_events, event_num, -1);

    if (n < 0) {
        Logger.Error("Epoller::Epoll", strerror(errno), "epoll_create1 failed");
    }

    for (int i = 0; i < n; i++) {
        Channel *chan = static_cast<Channel*>(epoll_events[i].data.ptr);
        int events = epoll_events[i].events;
        chan->SetReadyEvents(events);
        ready_channels.emplace_back(chan);
    }

    return ready_channels;
}

void Epoller::UndateChannel(Channel* chan) {
    int client_fd = chan->ClientFd();
    epoll_event event;
    event.events = chan->ListeningEvents();
    event.data.ptr = chan;
    
    if (!chan->IsInEpoll()) {
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
            Logger.Error("Epoller::UndateChannel", strerror(errno), "epoll_ctl ADD failed");
            chan->SetFailed();
            return;
        }

        chan->SetInEpoll(true);
    } else {
        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &event) == -1) {
            Logger.Error("Epoller::UndateChannel", strerror(errno), "epoll_ctl MOD failed");
            chan->SetFailed();
            return;
        }
    }
}

void Epoller::DeleteChannel(Channel *chan) {
    int client_fd = chan->ClientFd();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr) == -1) {
        Logger.Error("Epoller::UndateChannel", strerror(errno), "epoll_ctl DEL failed");
        chan->SetFailed();
    }
    chan->SetInEpoll(false);
}
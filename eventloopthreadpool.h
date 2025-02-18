#pragma once

#include "eventloop.h"
#include "eventloopthread.h"
#include <atomic>

class EventLoopThreadPool {
private:
    EventLoop *main_reactor;
    std::vector<std::unique_ptr<EventLoopThread>> sub_reactors;

    std::vector<EventLoop *> event_loops;

    int thread_nums;

    int next;
public:
    EventLoopThreadPool(EventLoop *event_loop);
    ~EventLoopThreadPool();

    void SetThreadNums(int thread_nums);

    void Start();

    EventLoop *NextEventLoop();
};
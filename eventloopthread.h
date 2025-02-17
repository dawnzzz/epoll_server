#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "eventloop.h"

class EventLoopThread {
private:
    EventLoop* event_loop;

    std::thread thread;
    std::mutex mu;
    std::condition_variable cv;
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* Start();
    void ThreadRunFunc();
};


#include <memory>

#include "eventloopthreadpool.h"
#include "logger.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *event_loop) : main_reactor(event_loop), thread_nums(0), next(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::Start() {
    Logger.Info("EventLoopThreadPool::Start", "", "thread pool starting...");

    for (int i = 0; i < thread_nums; i++) {
        std::unique_ptr<EventLoopThread> event_loop_thread = std::make_unique<EventLoopThread>();
        sub_reactors.push_back(std::move(event_loop_thread));
        event_loops.push_back(sub_reactors.back()->Start());
    }

    Logger.Info("EventLoopThreadPool::Start", "", "thread pooling start succ");
}

EventLoop* EventLoopThreadPool::NextEventLoop() {
    EventLoop* ret = main_reactor;
    if (!sub_reactors.empty()) {
        ret = event_loops[next++];
        if (next == sub_reactors.size()) {
            next = 0;
        }
    }

    return ret;
}

void EventLoopThreadPool::SetThreadNums(int thread_nums_) {
    thread_nums = thread_nums_;
}
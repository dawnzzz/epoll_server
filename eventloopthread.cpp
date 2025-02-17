

#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>
#include <sstream>
#include <mutex>

#include "eventloopthread.h"
#include "eventloop.h"
#include "currentthread.h"
#include "logger.h"

EventLoopThread::EventLoopThread() : event_loop(nullptr) {
}

EventLoopThread::~EventLoopThread() {
    thread.join();
}

EventLoop* EventLoopThread::Start() {
    
    thread = std::thread(std::bind(&EventLoopThread::ThreadRunFunc, this));
    
    EventLoop* current_loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mu);
        while (event_loop == nullptr) {
            cv.wait(lock);
        }

        current_loop = event_loop;
    }

    return current_loop;
}

void EventLoopThread::ThreadRunFunc() {
    std::ostringstream oss;
    oss << "Event Loop Thead[" << CurrentThread::gettid() << "] starting...";
    Logger.Info("EventLoopThread::ThreadRunFunc", "", oss.str().c_str());
    EventLoop loop1;
    // 由IO线程创建EventLoop对象
    EventLoop loop;

    {
        std::unique_lock<std::mutex> lock(mu);
        
        event_loop = &loop; // 获取子线程的地址
        
        cv.notify_one(); // loop_被创建成功，发送通知，唤醒主线程。
    }
    oss << "Event Loop Thead[" << CurrentThread::gettid() << "] start succ";
    Logger.Info("EventLoopThread::ThreadRunFunc", "", oss.str().c_str());
    event_loop->Start(); // 开始循环
    {
        std::unique_lock<std::mutex> lock(mu);
        event_loop = nullptr;
    }


}
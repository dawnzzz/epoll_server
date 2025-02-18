
#include <cstring>
#include <mutex>
#include <sstream>
#include <unistd.h>
#include <assert.h>
#include <memory>
#include <sys/eventfd.h>
#include <iostream>

#include "epoller.h"
#include "eventloop.h"
#include "channel.h"
#include "currentthread.h"
#include "logger.h"

EventLoop::EventLoop() {
    epoller = std::make_unique<Epoller>();
    wakeup_event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeup_event_fd == -1) {
        Logger.Fatal("EventLoop::EventLoop()", strerror(errno), "EventLoop create failed");
    }
    
    wakeup_channel = std::make_unique<Channel>(wakeup_event_fd, this);
    calling_funcs = false;
    wakeup_channel->SetReadCallBack(std::bind(&EventLoop::HandleRead, this));
    wakeup_channel->EnableRead();
}

EventLoop::~EventLoop() {
    DeleteChannel(wakeup_channel.get());
    close(wakeup_event_fd);
}

void EventLoop::UpdateChannel(Channel* chan) {
    epoller->UndateChannel(chan);
}

void EventLoop::DeleteChannel(Channel* chan) {
    epoller->DeleteChannel(chan);
}


void EventLoop::Start() {
    // std::ostringstream oss;
    
    while (true) {
        auto ready_channels = epoller->Epoll();
        if (ready_channels.size() > 0) {
            // oss << "get ready_channels num = " << ready_channels.size(); 
            // Logger.Info("EventLoop::Start", "", oss.str().c_str());
            // oss.str("");
        }
        for (auto chan : ready_channels) {
            // 处理事件
            chan->HandleEvent();
        }

        HandleToDoList();
    }
}

void EventLoop::RunOneFunc(std::function<void()> cb){
    if(IsInLoopThread()){
        cb();
    }else{
        QueueOneFunc(cb);
    }
}

bool EventLoop::IsInLoopThread(){
    return CurrentThread::tid() == tid_;
}

void EventLoop::QueueOneFunc(std::function<void()> cb){
    {
        std::unique_lock<std::mutex> lock(mu);
        to_do_list.emplace_back(std::move(cb));
    }

    // 如果调用当前函数的并不是当前当前EventLoop对应的的线程，将其唤醒。主要用于关闭TcpConnection
    // 由于关闭连接是由对应`TcpConnection`所发起的，但是关闭连接的操作应该由main_reactor所进行(为了释放ConnectionMap的所持有的TcpConnection)
    if (!IsInLoopThread() || calling_funcs) {
        uint64_t write_one_byte = 1;  
        ssize_t write_size = write(wakeup_event_fd, &write_one_byte, sizeof(write_one_byte));
    } 
}

void EventLoop::HandleToDoList(){
    // 此时已经epoll_wait出来，可能存在阻塞在epoll_wait的可能性。
    calling_funcs = true;

    std::vector< std::function<void()>> funcs;
    {
        // 加锁 保证线程同步
        std::unique_lock<std::mutex> lock(mu); 
        funcs.swap(to_do_list);
    }
    for(const auto& func: funcs){
        func();
    }

    calling_funcs = false;
}

void EventLoop::HandleRead(){
    uint64_t read_one_byte = 1;
    ssize_t read_size = read(wakeup_event_fd, &read_one_byte, sizeof(read_one_byte));
    (void) read_size;
    assert(read_size == sizeof(read_one_byte));
    return;
}

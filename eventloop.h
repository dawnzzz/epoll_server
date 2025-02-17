
#pragma  once

#include <memory>
#include <mutex>

#include "epoller.h"
#include "channel.h"

class EventLoop {
private:

public:
    EventLoop();
    ~EventLoop();

    void Start();

    void UpdateChannel(Channel *chan);
    void DeleteChannel(Channel *chan);

    // 判断调用该函数的是不是当前的线程，即是不是创建当前Loop的线程。
    bool IsInLoopThread();

    // 运行队列中的任务
    void HandleToDoList();

    // 将任务添加到队列中。当loop完成polling后运行
    void QueueOneFunc(std::function<void()> fn); 

    // 如果由创建本Loop的线程调用，则立即执行fn任务
    // 否则，将fn加入到队列中，等待之后运行
    void RunOneFunc(std::function<void()> fn);

    void HandleRead();

private:
    std::unique_ptr<Epoller> epoller;

    std::mutex mu;

    int wakeup_event_fd;
    std::unique_ptr<Channel> wakeup_channel;

    std::vector<std::function<void()>> to_do_list;
    bool calling_funcs;
    pid_t tid_;
};
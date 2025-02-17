#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <fstream>
#include <queue>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_ERROR 2
#define _LOG_LEVEL LOG_INFO

// 日志级别，暂时定义几种
// 后面优化为异步记录
enum LOG_LEVEL {
    DEBUG,
    INFO,
    ERROR,
    FATAL,
};

class LoggerInner {
private:
    bool is_async = false;
    std::queue<std::string> buffer;
    bool is_print_cout = true;
    std::ostringstream oss;
    std::atomic<bool> async_thread_running{false};
    std::thread async_thread;
    std::mutex mu;

    void AsyncThreadFunc(const char *file_path);
    bool async_thread_failed = false;
public:
    LoggerInner();
    ~LoggerInner();
    void SetIsPrintCout(bool is_print_cout_);
    void StartAasync(const char * file_path);
    void Log(LOG_LEVEL log_level, std::string name, const char  *err, std::string message);
    void Debug(std::string name, const char  *err, std::string message);
    void Info(std::string name, const char  *err, std::string message);
    void Error(std::string name, const char  *err, std::string message);
    void Fatal(std::string name, const char  *err, std::string message);
};

static LoggerInner Logger;
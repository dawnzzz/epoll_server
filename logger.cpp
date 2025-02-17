#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <stdarg.h>
#include <functional>
#include <unistd.h>
#include <fstream>

#include "currentthread.h"
#include "logger.h"

std::string getLogLevelName(LOG_LEVEL log_level) {
    switch (log_level) {
    case LOG_LEVEL::DEBUG :
    return "DEBUG";
    case LOG_LEVEL::INFO :
        return "INFO";
    case LOG_LEVEL::ERROR :
        return "ERROR";
    case LOG_LEVEL::FATAL :
        return "ERROR";
    default:
        return "UNKNOW";
    }
}

LoggerInner::LoggerInner() {
}

LoggerInner::~LoggerInner() {
}

void LoggerInner::StartAasync(const char *file_path) {
    if (is_async) {
        return;
    }
    is_async = true;

    
    async_thread = std::thread(std::bind(&LoggerInner::AsyncThreadFunc, this, file_path));
}

void LoggerInner::AsyncThreadFunc(const char *file_path) {

    std::ofstream target_stream(file_path);
    if (target_stream.fail()) {
        is_async = false;
        Error("LoggerInner::StartAasync", strerror(errno), "async thread start failed");
        async_thread_failed = true;
        return;
    }
    Info("LoggerInner::StartAasync", "", "async thread running");

    try {
        int max_size = 10000;    // 每次写入的日志最大数量
        while (true) {
            {
                std::unique_lock<std::mutex> lock(mu);
                int cnt = 0;
                while (cnt++ < max_size && !buffer.empty()) {
                    if (target_stream.fail()) {
                        continue;
                    }
                    target_stream << buffer.front() << std::endl;
                    buffer.pop();
                }
                target_stream.flush();
            }

            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    } catch (...) {
        async_thread_failed = true;
    }
}

void LoggerInner::Log(LOG_LEVEL log_level, std::string name, const char  *err, std::string message ) {
    if (log_level < _LOG_LEVEL) {
        return;
    }

    if (is_print_cout)
        // 打印到控制台
        std::cout << "LEVEL[" << getLogLevelName(log_level) << "] thread[" << CurrentThread::gettid() << "] name[" << name << "]" << " err:" << err << " message: " <<  message << std::endl;
    if (is_async && !async_thread_failed) {
        // 异步增加到缓冲区中
        {
            std::unique_lock<std::mutex> lock(mu);
            oss << "LEVEL[" << getLogLevelName(log_level) << "] name[" << name << "]" << " err:" << err << " message: " <<  message << std::endl;
            buffer.push(oss.str());
            oss.str("");
        }
    } 
    // else {
    //     if (!target_stream.fail()) {
    //         target_stream << "LEVEL[" << getLogLevelName(log_level) << "] name[" << name << "]" << " err:" << err << " message: " <<  message << std::endl;
    //     }
    // }
}

void LoggerInner::Debug(std::string name, const char  *err, std::string message) {
    LOG_LEVEL log_level = LOG_LEVEL::INFO;

    Log(log_level, name, err, message);
}

void LoggerInner::Info(std::string name, const char  *err, std::string message) {
    LOG_LEVEL log_level = LOG_LEVEL::INFO;
    // #if (_LOG_LEVEL == LOG_DEBUG)
    // log_level = LOG_LEVEL::DEBUG;
    // #elif (_LOG_LEVEL == LOG_ERROR)
    // log_level = LOG_LEVEL::ERROR;
    // #elif (_LOG_LEVEL == LOG_INFO)
    // log_level = LOG_LEVEL::INFO;
    // #endif

    Log(log_level, name, err, message);
}

void LoggerInner::Error(std::string name, const char* err, std::string message) {
    LOG_LEVEL log_level = LOG_LEVEL::INFO;
    // #if (_LOG_LEVEL == LOG_DEBUG)
    // log_level = LOG_LEVEL::DEBUG;
    // #elif (_LOG_LEVEL == LOG_ERROR)
    // log_level = LOG_LEVEL::ERROR;
    // #elif (_LOG_LEVEL == LOG_INFO)
    // log_level = LOG_LEVEL::INFO;
    // #endif

    Log(log_level, name, err, message);
}

void LoggerInner::Fatal(std::string name, const char  *err, std::string message) {
    LOG_LEVEL log_level = LOG_LEVEL::INFO;
    // #if (_LOG_LEVEL == LOG_DEBUG)
    // log_level = LOG_LEVEL::DEBUG;
    // #elif (_LOG_LEVEL == LOG_ERROR)
    // log_level = LOG_LEVEL::ERROR;
    // #elif (_LOG_LEVEL == LOG_INFO)
    // log_level = LOG_LEVEL::INFO;
    // #endif

    Log(log_level, name, err, message);
    
}
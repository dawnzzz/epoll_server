#pragma once

class Config
{
public:
    Config();
    ~Config(){};

    void parse_arg(int argc, char*argv[]);

    // 监听地址
    const char * ADDR;

    //端口号
    int PORT;

    // 进程数量
    int THREAD_NUM;

    // 是否异步写入日志
    int IS_ASYNC_LOGGER;

    // 日志文件
    const char * LOG_FILE;
};
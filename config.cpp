#include <thread>
#include <unistd.h>
#include <stdlib.h>

#include "config.h"


Config::Config(){
    // 监听地址
    ADDR = "0.0.0.0";

    //端口号,默认8080
    PORT = 8080;


    //sub_reactor数量
    THREAD_NUM = std::thread::hardware_concurrency();

    //日志写入方式，默认同步
    IS_ASYNC_LOGGER = 0;

    // 日志写入文件路径
    LOG_FILE = "";

}

void Config::parse_arg(int argc, char*argv[]){
    int opt;
    const char *str = "p:l:t:a:f:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            PORT = atoi(optarg);
            break;
        }
        case 'l':
        {
            ADDR = optarg;
            break;
        }
        case 't':
        {
            THREAD_NUM = atoi(optarg);
            break;
        }
        case 'a':
        {
            IS_ASYNC_LOGGER = atoi(optarg);
            break;
        }
        case 'f':
        {
            LOG_FILE = optarg;
            break;
        }
        default:
            break;
        }
    }
}
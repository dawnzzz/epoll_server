# 编译器
CXX = g++
# 编译选项
CXXFLAGS = -g -O2 -std=c++14 -Wall -Wextra -pthread
# 目标可执行文件
TARGET = echo_server client client_longconn
# 源文件
SRCS = logger.cpp acceptor.cpp buffer.cpp channel.cpp config.cpp epoller.cpp connection.cpp currentthread.cpp eventloop.cpp eventloopthread.cpp eventloopthreadpool.cpp  server.cpp echoserver.cpp client.cpp client_longconn.cpp
# 目标文件
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 生成可执行文件
echo_server: echoserver.o $(filter-out client.o client_longconn.o,$(OBJS))
	$(CXX) $(CXXFLAGS) -o $@ $^

client: client.o
	$(CXX) $(CXXFLAGS) -o $@ $^

client_longconn: client_longconn.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# 清理生成的文件
clean:
	rm -f $(OBJS) $(TARGET)

# 伪目标
.PHONY: all clean
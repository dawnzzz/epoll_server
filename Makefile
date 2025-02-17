# 编译器
CXX = g++
# 编译选项
CXXFLAGS = -g -O2 -std=c++14 -Wall -Wextra -pthread
# 目标可执行文件
TARGET = echo_server
# 源文件
SRCS = logger.cpp acceptor.cpp buffer.cpp channel.cpp config.cpp epoller.cpp connection.cpp currentthread.cpp eventloop.cpp eventloopthread.cpp eventloopthreadpool.cpp  server.cpp echoserver.cpp
# 目标文件
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 生成目标文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(OBJS) $(TARGET)

# 伪目标
.PHONY: all clean
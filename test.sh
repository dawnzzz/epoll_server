#!/bin/bash

# 定义要执行的命令
command="./client"
# 定义循环持续的总时间（秒）
total_time=259200
# 记录开始时间
start_time=$(date +%s)

while true; do
    # 计算已经过去的时间
    current_time=$(date +%s)
    elapsed_time=$((current_time - start_time))

    # 判断是否超过了总时间
    if [ $elapsed_time -ge $total_time ]; then
        break
    fi

    # 执行命令
    eval $command
    # 可以根据需要添加适当的延迟，避免命令执行过于频繁
done
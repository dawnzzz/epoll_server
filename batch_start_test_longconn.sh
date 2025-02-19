#!/bin/bash

# 检查参数是否存在且为正整数
if [ $# -ne 1 ] || ! [[ $1 =~ ^[0-9]+$ ]] || [ $1 -lt 1 ]; then
  echo "错误：请指定一个正整数作为要启动的进程数量。"
  echo "用法：$0 <进程数>"
  exit 1
fi

num_processes=$1
pid_file="/tmp/test_longconn_pids"

# 清空旧的 PID 文件
> "$pid_file"

echo "正在启动 $num_processes 个 test_longconn.sh 进程..."

# 启动指定数量的进程，记录 PID 并保存日志到 test_x.log
for ((i=1; i<=num_processes; i++)); do
  log_file="test_${i}.log"
  ./test_longconn.sh > "$log_file" 2>&1 &
  pid=$!
  echo "$pid" >> "$pid_file"
  echo "进程 $i 已启动，PID：$pid，日志：$log_file"
done

echo "所有进程已启动。PID 已保存到 $pid_file"

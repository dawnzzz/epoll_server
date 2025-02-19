#!/bin/bash

pid_file="/tmp/test_longconn_pids"

if [ ! -f "$pid_file" ]; then
  echo "错误：PID 文件 $pid_file 不存在。"
  exit 1
fi

echo "正在停止所有 test_longconn.sh 进程..."

# 读取 PID 文件并终止所有进程
while read pid; do
  if kill -0 "$pid" 2>/dev/null; then
    kill "$pid"
    echo "已终止进程 $pid"
  else
    echo "进程 $pid 不存在或已终止"
  fi
done < "$pid_file"

# 清理 PID 文件
rm -f "$pid_file"
echo "所有进程已停止。PID 文件已删除。"

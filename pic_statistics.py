import os
import re
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter

# 配置参数
LOG_BASE = './log'
PLOT_STYLE = 'ggplot'
COLORS = ['#2c7bb6', '#d7191c']  # 蓝红双色
OUTPUT_FILE = './performance.svg'
DPI = 96
PLOT_STYLE = 'dark_background'
FONT_CONFIG = {'fontname': 'DejaVu Sans', 'size': 12}  # 使用开源字体

def parse_log_file(log_path):
    """解析单个日志文件，返回多行结果的平均值"""
    response_times = []
    qps_values = []
    
    with open(log_path, 'r') as f:
        for line in f:
            # 匹配响应时间和QPS（兼容不同格式）

            
            rt_match = re.search(r'Average response time[:]?\s*([\d.]+)\s*us', line, re.I)
            qps_match = re.search(r'Requests per second[:]?\s*([\d.]+)', line, re.I)
            
            # if rt_match and qps_match:
            #     try:
            #         rt = float(rt_match.group(1))
            #         qps = float(qps_match.group(1))
            #         response_times.append(rt)
            #         qps_values.append(qps)
            #     except ValueError:
            #         continue
            if rt_match:
                rt = float(rt_match.group(1))
                response_times.append(rt)
            if qps_match:
                qps = float(qps_match.group(1))
                qps_values.append(qps)
    
    # 计算文件平均值
    if response_times and qps_values:
        return {
            'avg_response': sum(response_times)/len(response_times),
            'avg_qps': sum(qps_values)/len(qps_values)
        }
    return None

def process_logs():
    """处理所有日志数据"""
    dataset = []
    
    # 遍历不同并发数的测试目录
    for concurrency in sorted([d for d in os.listdir(LOG_BASE) if d.isdigit()], 
                             key=lambda x: int(x)):
        dir_path = os.path.join(LOG_BASE, concurrency)
        if not os.path.isdir(dir_path):
            continue
        
        process_data = {
            'concurrency': int(concurrency),
            'response_times': [],
            'qps_values': []
        }
        
        # 处理每个测试脚本
        for log_file in sorted(os.listdir(dir_path)):
            if log_file.startswith('test_') and log_file.endswith('.log'):
                result = parse_log_file(os.path.join(dir_path, log_file))
                if result:
                    process_data['response_times'].append(result['avg_response'])
                    process_data['qps_values'].append(result['avg_qps'])
        
        # 计算该并发数下的聚合指标
        if process_data['response_times'] and process_data['qps_values']:
            dataset.append({
                'concurrency': process_data['concurrency'],
                'mean_response': sum(process_data['response_times'])/len(process_data['response_times']),
                'total_qps': sum(process_data['qps_values'])  # 总QPS为各进程QPS之和
            })
    
    return sorted(dataset, key=lambda x: x['concurrency'])

def format_thousands(x, pos):
    """千分位格式化"""
    return f"{x:,.0f}" if x < 1e4 else f"{x/1e3:,.0f}k"

def visualize_results(dataset):
    """专业级可视化设计"""
    plt.style.use(PLOT_STYLE)
    fig, ax1 = plt.subplots(figsize=(13, 7.5))
    
    # 设置全局背景
    fig.patch.set_facecolor('white')
    ax1.set_facecolor('white')
    
    x = [d['concurrency'] for d in dataset]
    responses = [d['mean_response'] for d in dataset]
    qps = [d['total_qps'] for d in dataset]
    
    # ====== 响应时间曲线 ======
    response_line = ax1.plot(
        x, responses,
        color=COLORS[0],
        marker='o',
        markersize=5,
        linewidth=2,
        linestyle='--',
        alpha=0.9,
        label='Response Time',
        zorder=3
    )
    
    # ====== QPS曲线 ======
    ax2 = ax1.twinx()
    qps_line = ax2.plot(
        x, qps,
        color=COLORS[1],
        marker='s',
        markersize=5,
        linewidth=2,
        linestyle='-.',
        alpha=0.9,
        label='Throughput (QPS)',
        zorder=3
    )
    
    # ====== 坐标轴配置 ======
    ax1.xaxis.set_minor_locator(plt.MultipleLocator(1))
    ax1.set_xticks(x)
    ax1.set_xticklabels(
        x,
        ha='center',   # 右端对齐
        fontsize=10,
        color='#444444'
    )
    ax1.set_xlabel('Concurrent Test Processes', **FONT_CONFIG)
    ax1.set_ylabel('Response Time (us)', color=COLORS[0], **FONT_CONFIG)
    ax2.set_ylabel('QPS', color=COLORS[1], **FONT_CONFIG)
    
    # 数值格式化
    ax1.yaxis.set_major_formatter(FuncFormatter(format_thousands))
    ax2.yaxis.set_major_formatter(FuncFormatter(format_thousands))
    
    # ====== 网格系统 ======
    ax1.grid(
        visible=True,
        axis='both',
        linestyle=':',
        linewidth=0.8,
        alpha=0.6,
        color='#666666',
        zorder=1
    )
    
    # ====== 刻度配置 ======
    ax1.tick_params(axis='y', labelcolor=COLORS[0], labelsize=10)
    ax2.tick_params(axis='y', labelcolor=COLORS[1], labelsize=10)
    ax1.tick_params(axis='x', labelsize=10)
    ax1.set_xticks(x)
    
    # ====== 数据标签 ======
    for xi, rt, q in zip(x, responses, qps):
        ax1.annotate(
            f'{rt:.1f}us',
            (xi, rt),
            textcoords="offset points",
            xytext=(0,10),
            ha='center',
            fontsize=6,
            color=COLORS[0],
            bbox=dict(
                boxstyle="round,pad=0.3",
                facecolor='white',
                edgecolor=COLORS[0],
                alpha=0.8
            )
        )
        
        ax2.annotate(
            f'{q:,.0f}',
            (xi, q),
            textcoords="offset points",
            xytext=(0,-15),
            ha='center',
            fontsize=6,
            color=COLORS[1],
            bbox=dict(
                boxstyle="round,pad=0.3",
                facecolor='white',
                edgecolor=COLORS[1],
                alpha=0.8
            )
        )
    
    # ====== 图例系统 ======
    # lines = response_line + qps_line
    # labels = [l.get_label() for l in lines]
    # ax1.legend(
    #     lines, labels,
    #     loc='upper left',
    #     bbox_to_anchor=(0.02, 0.96),
    #     frameon=True,
    #     framealpha=0.95,
    #     facecolor='white',
    #     edgecolor='#dddddd',
    #     fontsize=10
    # )
    
    # ====== 标题配置 ======
    # plt.title(
    #     "Enterprise-Grade Performance Analysis\nResponse Time vs System Throughput",
    #     fontdict={**FONT_CONFIG, 'size':14, 'weight':'bold'},
    #     pad=25,
    #     loc='left'
    # )
    
    # ====== 输出配置 ======
    plt.tight_layout(pad=4)
    plt.savefig(
        OUTPUT_FILE,
        dpi=DPI,
        bbox_inches='tight',
        facecolor='white',
        edgecolor='none'
    )
    plt.close()

# 执行分析
if __name__ == "__main__":
    test_data = process_logs()
    if test_data:
        visualize_results(test_data)
    else:
        print("错误：未找到有效的测试数据")

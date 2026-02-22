#!/bin/bash

# 网络带宽监控脚本
# 实时监控网络接口的带宽使用情况

INTERFACE="lo"  # 本地回环接口
SAMPLE_INTERVAL="1s"  # 采样间隔

# 函数：获取网络接口的带宽使用情况
get_bandwidth() {
    local interface=$1
    
    # 获取当前时间的流量统计
    current_stats=$(cat /proc/net/dev | grep $interface | awk '{print $2, $10}')
    read rx_bytes tx_bytes <<< $current_stats
    
    echo "$rx_bytes $tx_bytes"
}

# 主函数：监控带宽
monitor_bandwidth() {
    echo "Monitoring bandwidth on interface $INTERFACE..."
    echo "Press Ctrl+C to stop"
    echo ""
    echo "Time			Rx Bandwidth	Tx Bandwidth"
    echo "--------------------------------------------------"
    
    # 获取初始统计数据
    read rx_start tx_start <<< $(get_bandwidth $INTERFACE)
    prev_time=$(date +%s)
    
    while true; do
        # 等待采样间隔
        sleep $SAMPLE_INTERVAL
        
        # 获取当前统计数据
        read rx_end tx_end <<< $(get_bandwidth $INTERFACE)
        current_time=$(date +%s)
        
        # 计算时间差（秒）
        time_diff=$((current_time - prev_time))
        if [ $time_diff -eq 0 ]; then
            time_diff=1
        fi
        
        # 计算带宽（字节/秒）
        rx_bps=$(( (rx_end - rx_start) / time_diff ))
        tx_bps=$(( (tx_end - tx_start) / time_diff ))
        
        # 转换为人类可读格式
        rx_kbps=$(echo "scale=2; $rx_bps / 1024" | bc)
        tx_kbps=$(echo "scale=2; $tx_bps / 1024" | bc)
        rx_mbps=$(echo "scale=2; $rx_kbps / 1024" | bc)
        tx_mbps=$(echo "scale=2; $tx_kbps / 1024" | bc)
        
        # 输出结果
        timestamp=$(date +"%Y-%m-%d %H:%M:%S")
        if (( $(echo "$rx_mbps > 1" | bc -l) )); then
            rx_display="${rx_mbps} Mbps"
        else
            rx_display="${rx_kbps} Kbps"
        fi
        
        if (( $(echo "$tx_mbps > 1" | bc -l) )); then
            tx_display="${tx_mbps} Mbps"
        else
            tx_display="${tx_kbps} Kbps"
        fi
        
        echo "$timestamp	$rx_display	$tx_display"
        
        # 更新变量
        rx_start=$rx_end
        tx_start=$tx_end
        prev_time=$current_time
    done
}

# 运行主函数
monitor_bandwidth

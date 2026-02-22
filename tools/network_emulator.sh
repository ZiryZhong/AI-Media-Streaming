#!/bin/bash

# 网络波动模拟器脚本
# 使用tc和netem工具模拟网络延迟、丢包和带宽限制

INTERFACE="lo"  # 本地回环接口
BASE_DELAY="10ms"  # 基础延迟
MAX_JITTER="5ms"  # 最大抖动
LOSS_RATE="0.1%"  # 丢包率
BASE_BANDWIDTH="100mbit"  # 基础带宽
MIN_BANDWIDTH="1mbit"  # 最小带宽

# 检测是否为WSL2环境
is_wsl2() {
    grep -q "microsoft" /proc/version
    return $?
}

# 函数：检查tc和netem是否可用
is_tc_available() {
    sudo tc qdisc help >/dev/null 2>&1
    return $?
}

# 函数：设置网络参数
set_network_params() {
    local delay=$1
    local loss=$2
    local bandwidth=$3
    
    echo "Setting network parameters: delay=$delay, loss=$loss, bandwidth=$bandwidth"
    
    # 检查tc是否可用
    if is_tc_available; then
        # 清除现有规则
        sudo tc qdisc del dev $INTERFACE root 2>/dev/null
        
        # 检查是否为WSL2环境
        if is_wsl2; then
            echo "Warning: Running in WSL2 environment. Some network emulation features may be limited."
            echo "Trying to use basic netem without htb..."
            # 在WSL2中，尝试只使用netem（如果可用）
            sudo tc qdisc add dev $INTERFACE root netem delay $delay loss $loss 2>/dev/null
            if [ $? -ne 0 ]; then
                echo "Info: netem is not available in this WSL2 environment."
                echo "Using software-based network fluctuation simulation..."
            fi
        else
            # 在正常Linux环境中，使用完整的htb和netem
            # 首先添加htb根队列
            sudo tc qdisc add dev $INTERFACE root handle 1: htb default 10
            # 添加带宽限制类
            sudo tc class add dev $INTERFACE parent 1: classid 1:10 htb rate $bandwidth ceil $bandwidth
            # 添加netem子队列用于模拟延迟和丢包
            sudo tc qdisc add dev $INTERFACE parent 1:10 handle 10: netem delay $delay loss $loss
        fi
    else
        echo "Info: tc command is not available."
        echo "Using software-based network fluctuation simulation..."
    fi
}

# 函数：软件模拟网络波动
software_based_fluctuation() {
    echo "Starting software-based network fluctuation simulation..."
    echo "This will simulate bandwidth changes by logging and providing network condition data."
    
    while true; do
        # 随机生成网络参数
        jitter=$(awk -v max="$MAX_JITTER" 'BEGIN{srand(); print rand() * max}')
        delay="$BASE_DELAY $(printf "%.2f" $jitter)ms"
        
        loss=$(awk -v base="$LOSS_RATE" 'BEGIN{srand(); print rand() * 2 * base}')
        loss=$(printf "%.2f%%" $loss)
        
        bandwidth=$(awk -v min="$MIN_BANDWIDTH" -v base="$BASE_BANDWIDTH" 'BEGIN{srand(); print rand() * (100 - 1) + 1}')
        bandwidth=$(printf "%.0f" $bandwidth)"mbit"
        
        echo "[Software Simulation] Network conditions: delay=$delay, loss=$loss, bandwidth=$bandwidth"
        
        # 等待一段时间
        sleep 5
    done
}

# 函数：模拟网络波动
emulate_network_fluctuations() {
    echo "Starting network fluctuation simulation..."
    
    # 检查tc是否可用
    if is_tc_available; then
        # 检查是否为WSL2环境
        if is_wsl2; then
            # 尝试使用netem
            sudo tc qdisc add dev $INTERFACE root netem delay "$BASE_DELAY" loss "$LOSS_RATE" 2>/dev/null
            if [ $? -ne 0 ]; then
                # netem不可用，使用软件模拟
                software_based_fluctuation
                return
            fi
        fi
        
        # 使用tc和netem模拟网络波动
        while true; do
            # 随机生成网络参数
            jitter=$(awk -v max="$MAX_JITTER" 'BEGIN{srand(); print rand() * max}')
            delay="$BASE_DELAY $(printf "%.2f" $jitter)ms"
            
            loss=$(awk -v base="$LOSS_RATE" 'BEGIN{srand(); print rand() * 2 * base}')
            loss=$(printf "%.2f%%" $loss)
            
            bandwidth=$(awk -v min="$MIN_BANDWIDTH" -v base="$BASE_BANDWIDTH" 'BEGIN{srand(); print rand() * (100 - 1) + 1}')
            bandwidth=$(printf "%.0f" $bandwidth)"mbit"
            
            # 设置网络参数
            set_network_params "$delay" "$loss" "$bandwidth"
            
            # 等待一段时间
            sleep 5
        done
    else
        # tc不可用，使用软件模拟
        software_based_fluctuation
    fi
}

# 函数：重置网络参数
reset_network() {
    echo "Resetting network parameters..."
    sudo tc qdisc del dev $INTERFACE root 2>/dev/null
    echo "Network parameters reset to default"
}

# 主菜单
main() {
    case "$1" in
        "start")
            emulate_network_fluctuations
            ;;
        "stop")
            reset_network
            ;;
        "reset")
            reset_network
            ;;
        *)
            echo "Usage: $0 {start|stop|reset}"
            echo "  start: 开始网络波动模拟"
            echo "  stop: 停止网络波动模拟"
            echo "  reset: 重置网络参数为默认值"
            ;;
    esac
}

# 检查权限
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root or use sudo"
    exit 1
fi

# 运行主函数
main "$1"

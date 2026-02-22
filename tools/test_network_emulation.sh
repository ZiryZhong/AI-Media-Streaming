#!/bin/bash

# 网络波动测试脚本
# 验证网络波动模拟器是否生效

# 脚本路径
NETWORK_EMULATOR="./network_emulator.sh"
BANDWIDTH_MONITOR="./bandwidth_monitor.sh"
PERFORMANCE_TEST="../build/performance_test"

# 测试参数
FRAME_COUNT=50
WIDTH=640
HEIGHT=480

# 函数：启动网络波动模拟器
start_network_emulator() {
    echo "Starting network emulator..."
    $NETWORK_EMULATOR start > network_emulator.log 2>&1 &
    EMULATOR_PID=$!
    echo "Network emulator started with PID: $EMULATOR_PID"
}

# 函数：停止网络波动模拟器
stop_network_emulator() {
    echo "Stopping network emulator..."
    $NETWORK_EMULATOR stop > /dev/null 2>&1
    if [ -n "$EMULATOR_PID" ]; then
        kill -SIGTERM $EMULATOR_PID 2>/dev/null
    fi
    echo "Network emulator stopped"
}

# 函数：启动带宽监控
start_bandwidth_monitor() {
    echo "Starting bandwidth monitor..."
    $BANDWIDTH_MONITOR > bandwidth_monitor.log 2>&1 &
    MONITOR_PID=$!
    echo "Bandwidth monitor started with PID: $MONITOR_PID"
}

# 函数：停止带宽监控
stop_bandwidth_monitor() {
    echo "Stopping bandwidth monitor..."
    if [ -n "$MONITOR_PID" ]; then
        kill -SIGTERM $MONITOR_PID 2>/dev/null
    fi
    echo "Bandwidth monitor stopped"
}

# 主函数：运行测试
run_test() {
    echo "=== Network Emulation Test ==="
    echo "Test parameters: $FRAME_COUNT frames, $WIDTH x $HEIGHT resolution"
    echo "=============================="
    
    # 重置网络参数
    $NETWORK_EMULATOR reset
    
    # 启动网络波动模拟器
    start_network_emulator
    
    # 等待网络波动模拟器启动
    sleep 2
    
    # 启动带宽监控
    start_bandwidth_monitor
    
    # 等待带宽监控启动
    sleep 2
    
    # 运行性能测试
    echo "Running performance test..."
    $PERFORMANCE_TEST $FRAME_COUNT $WIDTH $HEIGHT > performance_test.log 2>&1
    
    # 等待测试完成
    sleep 5
    
    # 停止监控和模拟器
    stop_bandwidth_monitor
    stop_network_emulator
    
    # 重置网络参数
    $NETWORK_EMULATOR reset
    
    echo ""
    echo "=== Test Results ==="
    echo "Network emulator log:"
    tail -20 network_emulator.log
    echo ""
    echo "Bandwidth monitor log (last 20 lines):"
    tail -20 bandwidth_monitor.log
    echo ""
    echo "Performance test log:"
    cat performance_test.log
    echo ""
    echo "=== Test Completed ==="
}

# 设置脚本执行权限
chmod +x $NETWORK_EMULATOR
chmod +x $BANDWIDTH_MONITOR

# 检查性能测试可执行文件是否存在
if [ ! -f $PERFORMANCE_TEST ]; then
    echo "Error: Performance test executable not found at $PERFORMANCE_TEST"
    echo "Please build the project first"
    exit 1
fi

# 运行测试
run_test

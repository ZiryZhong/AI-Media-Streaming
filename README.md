# StreamingNetworkFramework

一个简易的流媒体网络传输框架，支持AI-based的码率控制算法和自定义编解码器。

## 功能特性

- **插件式架构**：支持插入AI码率控制算法和自定义编解码器
- **UDP网络传输**：实现了真实的UDP socket传输，支持本地模拟
- **FFmpeg集成**：集成了FFmpeg编解码器功能，支持YUV文件传输
- **网络波动模拟**：提供网络波动模拟工具，使用tc和netem模拟带宽变化
- **性能测试**：包含性能测试程序，测试媒体传输性能
- **日志系统**：实现了日志模块，监控流媒体传输的关键步骤

## 项目结构

```
StreamingNetworkFramework/
├── CMakeLists.txt       # 构建配置文件
├── include/             # 头文件目录
├── src/                 # 源代码目录
│   ├── core/            # 核心模块
│   │   ├── stream_manager.cpp     # 流会话管理
│   │   ├── stream_session.cpp     # 流会话实现
│   │   └── logger.cpp             # 日志模块
│   ├── network/         # 网络层
│   │   ├── network_transport.cpp   # 网络传输实现
│   │   └── packet_manager.cpp      # 数据包管理
│   └── plugins/         # 插件系统
│       ├── rate_control/           # 码率控制插件
│       └── codec/                  # 编解码器插件
├── examples/            # 示例代码
│   └── test/            # 测试程序
├── tools/               # 工具脚本
│   ├── network_emulator.sh         # 网络波动模拟脚本
│   ├── bandwidth_monitor.sh        # 带宽监控脚本
│   └── test_network_emulation.sh   # 网络波动测试脚本
├── build/               # 构建目录
└── videos/              # 视频文件目录
```

## 系统要求

- C++14 或更高版本
- CMake 3.10 或更高版本
- FFmpeg 库
- Linux 系统（网络波动模拟工具需要tc和netem）

## 构建项目

1. **克隆仓库**

```bash
git clone git@github.com:ZiryZhong/AI-Media-Streaming.git
cd AI-Media-Streaming
```

2. **创建构建目录**

```bash
mkdir -p build
cd build
```

3. **配置项目**

```bash
cmake ..
```

4. **构建项目**

```bash
make -j4
```

## 使用方法

### 1. 运行基本测试

```bash
./test_streaming
```

### 2. 运行FFmpeg编解码测试

```bash
./test_ffmpeg_streaming
```

### 3. 运行UDP服务器

```bash
./udp_server
```

### 4. 运行性能测试

```bash
./performance_test <frame_count> <width> <height>
```

例如：

```bash
./performance_test 50 640 480
```

### 5. 使用网络波动模拟工具

**启动网络波动模拟**：

```bash
sudo ./tools/network_emulator.sh start
```

**停止网络波动模拟**：

```bash
sudo ./tools/network_emulator.sh stop
```

**测试网络波动模拟**：

```bash
sudo ./tools/test_network_emulation.sh
```

## 插件系统

### 码率控制插件

- **example_ai_rate_control**：示例AI码率控制算法

### 编解码器插件

- **example_custom_codec**：示例自定义编解码器
- **ffmpeg_codec**：基于FFmpeg的编解码器实现

## 网络波动模拟

网络波动模拟工具使用tc和netem工具来模拟网络延迟、丢包和带宽限制。在WSL2环境中，会自动切换到软件模拟模式。

### 模拟参数

- **基础延迟**：10ms
- **最大抖动**：5ms
- **丢包率**：0.1%
- **基础带宽**：100mbit
- **最小带宽**：1mbit

### 带宽监控

使用带宽监控脚本实时监控网络接口的带宽使用情况：

```bash
./tools/bandwidth_monitor.sh
```

## 性能测试

性能测试程序会测试以下指标：

- **总帧数**：处理的视频帧总数
- **总时间**：处理所有帧的总时间
- **平均帧时间**：每帧的平均处理时间
- **吞吐量**：数据传输的吞吐量
- **延迟**：数据传输的延迟

## 日志系统

日志系统会记录以下关键步骤：

- 流会话的创建和销毁
- 网络传输的发送和接收
- 编解码器的初始化和帧处理
- 码率控制算法的决策过程

## 技术栈

- **C++14**：核心编程语言
- **CMake**：构建系统
- **FFmpeg**：视频编解码库
- **UDP Socket**：网络传输协议
- **tc/netem**：网络波动模拟工具

## 许可证

MIT License

## 贡献

欢迎提交Issue和Pull Request！

## 联系方式

- GitHub: [ZiryZhong](https://github.com/ZiryZhong)

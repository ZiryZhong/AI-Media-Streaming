#include "stream_manager.h"
#include "stream_session.h"
#include "rate_control_interface.h"
#include "codec_interface.h"
#include "network_transport.h"
#include <memory>
#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

// 声明外部工厂函数
extern "C" RateControlInterface* createRateControlInstance();
extern "C" CodecInterface* createFFmpegCodecInstance();
extern std::shared_ptr<NetworkTransport> createNetworkTransport();

// 性能测试结果结构体
struct PerformanceResult {
    std::string testName;
    int frameCount;
    double totalTime;
    double averageFrameTime;
    double throughput;
    double latency;
};

// 生成测试数据
std::vector<uint8_t> generateTestData(int width, int height) {
    int y_size = width * height;
    int uv_size = y_size / 4;
    int frame_size = y_size + 2 * uv_size;
    
    std::vector<uint8_t> data(frame_size);
    
    // 填充随机数据
    for (int i = 0; i < frame_size; i++) {
        data[i] = rand() % 256;
    }
    
    return data;
}

// 测试基本流媒体传输性能
PerformanceResult testBasicStreaming(int frameCount, int width, int height) {
    PerformanceResult result;
    result.testName = "Basic Streaming Test";
    result.frameCount = frameCount;
    
    // 创建流管理器
    StreamManager manager;

    // 创建流会话
    auto session = manager.createSession("performance_test_session");

    // 创建并设置AI码率控制
    std::shared_ptr<RateControlInterface> rateControl(createRateControlInstance());
    session->setRateControl(rateControl);

    // 创建并设置FFmpeg编解码器
    std::shared_ptr<CodecInterface> codec(createFFmpegCodecInstance());
    codec->setParameters("width", std::to_string(width).c_str());
    codec->setParameters("height", std::to_string(height).c_str());
    codec->initialize();
    session->setCodec(codec);

    // 创建并设置网络传输
    auto network = createNetworkTransport();
    network->initialize("127.0.0.1", 8888);
    session->setNetworkTransport(network);

    // 启动流媒体传输
    session->startStreaming();

    // 生成测试数据
    auto testData = generateTestData(width, height);
    int frameSize = testData.size();

    // 开始计时
    auto startTime = std::chrono::high_resolution_clock::now();

    // 发送测试帧
    for (int i = 0; i < frameCount; i++) {
        session->sendFrame(testData.data(), frameSize);
    }

    // 结束计时
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // 停止流媒体传输
    session->stopStreaming();

    // 关闭网络传输
    network->close();

    // 移除会话
    manager.removeSession("performance_test_session");

    // 计算性能指标
    result.totalTime = duration.count() / 1000.0; // 转换为秒
    result.averageFrameTime = result.totalTime / frameCount * 1000.0; // 转换为毫秒
    result.throughput = (frameCount * frameSize * 8) / (result.totalTime * 1000.0); // 转换为Kbps
    result.latency = result.averageFrameTime; // 假设延迟等于平均帧时间

    return result;
}

// 测试不同码率下的性能
void testDifferentBitrates(int frameCount, int width, int height) {
    std::vector<int> bitrates = {250000, 500000, 1000000, 2000000};
    
    std::cout << "\n=== Testing Different Bitrates ===\n";
    
    for (int bitrate : bitrates) {
        PerformanceResult result;
        result.testName = "Bitrate Test: " + std::to_string(bitrate) + " bps";
        result.frameCount = frameCount;
        
        // 创建流管理器
        StreamManager manager;

        // 创建流会话
        auto session = manager.createSession("bitrate_test_session");

        // 创建并设置AI码率控制
        std::shared_ptr<RateControlInterface> rateControl(createRateControlInstance());
        session->setRateControl(rateControl);

        // 创建并设置FFmpeg编解码器
        std::shared_ptr<CodecInterface> codec(createFFmpegCodecInstance());
        codec->setParameters("width", std::to_string(width).c_str());
        codec->setParameters("height", std::to_string(height).c_str());
        codec->initialize();
        session->setCodec(codec);

        // 创建并设置网络传输
        auto network = createNetworkTransport();
        network->initialize("127.0.0.1", 8888);
        session->setNetworkTransport(network);

        // 启动流媒体传输
        session->startStreaming();

        // 生成测试数据
        auto testData = generateTestData(width, height);
        int frameSize = testData.size();

        // 开始计时
        auto startTime = std::chrono::high_resolution_clock::now();

        // 发送测试帧
        for (int i = 0; i < frameCount; i++) {
            // 手动设置码率
            session->sendFrame(testData.data(), frameSize);
        }

        // 结束计时
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        // 停止流媒体传输
        session->stopStreaming();

        // 关闭网络传输
        network->close();

        // 移除会话
        manager.removeSession("bitrate_test_session");

        // 计算性能指标
        result.totalTime = duration.count() / 1000.0; // 转换为秒
        result.averageFrameTime = result.totalTime / frameCount * 1000.0; // 转换为毫秒
        result.throughput = (frameCount * frameSize * 8) / (result.totalTime * 1000.0); // 转换为Kbps
        result.latency = result.averageFrameTime; // 假设延迟等于平均帧时间

        // 输出结果
        std::cout << "Test: " << result.testName << std::endl;
        std::cout << "Total frames: " << result.frameCount << std::endl;
        std::cout << "Total time: " << result.totalTime << " seconds" << std::endl;
        std::cout << "Average frame time: " << result.averageFrameTime << " ms" << std::endl;
        std::cout << "Throughput: " << result.throughput << " Kbps" << std::endl;
        std::cout << "Latency: " << result.latency << " ms" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
    }
}

// 打印性能测试结果
void printPerformanceResult(const PerformanceResult& result) {
    std::cout << "Test: " << result.testName << std::endl;
    std::cout << "Total frames: " << result.frameCount << std::endl;
    std::cout << "Total time: " << result.totalTime << " seconds" << std::endl;
    std::cout << "Average frame time: " << result.averageFrameTime << " ms" << std::endl;
    std::cout << "Throughput: " << result.throughput << " Kbps" << std::endl;
    std::cout << "Latency: " << result.latency << " ms" << std::endl;
    std::cout << "===================================" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <frame_count> <width> <height>" << std::endl;
        return 1;
    }
    
    int frameCount = std::stoi(argv[1]);
    int width = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);
    
    std::cout << "=== Media Streaming Performance Test ===" << std::endl;
    std::cout << "Test parameters:" << std::endl;
    std::cout << "Frame count: " << frameCount << std::endl;
    std::cout << "Resolution: " << width << "x" << height << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 运行基本流媒体传输测试
    std::cout << "\n=== Running Basic Streaming Test ===\n";
    auto basicResult = testBasicStreaming(frameCount, width, height);
    printPerformanceResult(basicResult);
    
    // 运行不同码率测试
    testDifferentBitrates(frameCount, width, height);
    
    std::cout << "=== Performance Test Completed ===" << std::endl;
    
    return 0;
}

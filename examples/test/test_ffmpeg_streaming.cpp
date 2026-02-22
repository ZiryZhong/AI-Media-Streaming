#include "stream_manager.h"
#include "stream_session.h"
#include "rate_control_interface.h"
#include "codec_interface.h"
#include "network_transport.h"
#include <memory>
#include <fstream>
#include <iostream>
#include <unistd.h>

// 声明外部工厂函数
extern "C" RateControlInterface* createRateControlInstance();
extern "C" CodecInterface* createFFmpegCodecInstance();
extern std::shared_ptr<NetworkTransport> createNetworkTransport();

// 读取YUV文件数据
std::vector<uint8_t> readYUVFile(const std::string& filename, int width, int height) {
    std::vector<uint8_t> data;
    std::ifstream file(filename, std::ios::binary);
    
    if (!file.is_open()) {
        return data;
    }
    
    // 计算一帧YUV420P数据的大小
    int y_size = width * height;
    int uv_size = y_size / 4;
    int frame_size = y_size + 2 * uv_size;
    
    // 读取文件数据
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    data.resize(file_size);
    file.read(reinterpret_cast<char*>(data.data()), file_size);
    
    file.close();
    return data;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <yuv_file> <width> <height>" << std::endl;
        return 1;
    }
    
    std::string yuv_file = argv[1];
    int width = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);
    
    // 创建流管理器
    StreamManager manager;

    // 创建流会话
    auto session = manager.createSession("ffmpeg_test_session");

    // 创建并设置AI码率控制
    std::shared_ptr<RateControlInterface> rateControl(createRateControlInstance());
    session->setRateControl(rateControl);

    // 创建并设置FFmpeg编解码器
    std::shared_ptr<CodecInterface> codec(createFFmpegCodecInstance());
    // 设置编解码器参数
    codec->setParameters("width", argv[2]);
    codec->setParameters("height", argv[3]);
    // 初始化编解码器
    codec->initialize();
    session->setCodec(codec);

    // 创建并设置网络传输
    auto network = createNetworkTransport();
    // 初始化网络传输，使用本地地址和端口8888
    network->initialize("127.0.0.1", 8888);
    session->setNetworkTransport(network);

    // 启动流媒体传输
    session->startStreaming();

    // 读取YUV文件数据
    auto yuv_data = readYUVFile(yuv_file, width, height);
    if (yuv_data.empty()) {
        std::cout << "Failed to read YUV file: " << yuv_file << std::endl;
        return 1;
    }
    
    // 计算一帧YUV420P数据的大小
    int y_size = width * height;
    int uv_size = y_size / 4;
    int frame_size = y_size + 2 * uv_size;
    
    // 发送YUV数据（假设文件包含多帧）
    int num_frames = yuv_data.size() / frame_size;
    std::cout << "Found " << num_frames << " frames in YUV file" << std::endl;
    
    for (int i = 0; i < num_frames; i++) {
        std::cout << "Sending frame " << (i + 1) << "/" << num_frames << std::endl;
        
        // 获取当前帧的数据
        const uint8_t* frame_data = yuv_data.data() + i * frame_size;
        
        // 发送帧数据
        session->sendFrame(frame_data, frame_size);
        
        // 短暂延迟，模拟实时流
        usleep(40000); // 25fps
    }

    // 停止流媒体传输
    session->stopStreaming();

    // 关闭网络传输
    network->close();

    // 移除会话
    manager.removeSession("ffmpeg_test_session");

    return 0;
}

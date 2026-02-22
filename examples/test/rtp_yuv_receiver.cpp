#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstring>
#include <vector>
#include <map>
#include <arpa/inet.h>
#include "rtp_transport.h"
#include "logger.h"

// 测试RTP YUV接收功能
int main(int argc, char* argv[]) {
    LOG_INFO("Starting RTP YUV receiver");
    
    try {
        // 检查参数
        if (argc != 5) {
            LOG_ERROR("Usage: rtp_yuv_receiver <output_yuv_file> <width> <height> <max_frames>");
            return 1;
        }
        
        std::string output_file = argv[1];
        int width = std::stoi(argv[2]);
        int height = std::stoi(argv[3]);
        int max_frames = std::stoi(argv[4]);
        
        // 计算一帧的大小（YUV420P格式）
        size_t frame_size = width * height * 3 / 2;
        LOG_INFO("Frame size: " + std::to_string(frame_size) + " bytes");
        
        // 打开输出文件
        std::ofstream output_stream(output_file, std::ios::binary);
        if (!output_stream) {
            LOG_ERROR("Failed to open output YUV file: " + output_file);
            return 1;
        }
        
        // 创建RTP传输实例
        RtpTransport rtp_transport;
        
        // 初始化RTP传输
        rtp_transport.initialize("127.0.0.1", 8889); // 使用不同的端口
        
        // 设置负载类型为H.264
        rtp_transport.setPayloadType(96);
        
        // 用于存储接收到的数据包
        std::vector<uint8_t> current_frame;
        current_frame.reserve(frame_size);
        
        // 用于跟踪数据包的序列号
        uint16_t expected_sequence = 0;
        bool first_packet = true;
        
        int frame_count = 0;
        bool running = true;
        
        // 接收数据包
        while (running && frame_count < max_frames) {
            // 分配缓冲区
            uint8_t buffer[RTP_MAX_PACKET_SIZE];
            size_t received_size = 0;
            
            // 接收RTP包
            rtp_transport.receiveRtpPacket(buffer, RTP_MAX_PACKET_SIZE, received_size);
            
            if (received_size > 0) {
                // 解析RTP头部
                RtpHeader* header = reinterpret_cast<RtpHeader*>(buffer);
                uint16_t sequence = ntohs(header->sequence);
                
                // 计算负载大小
                size_t payload_size = received_size - sizeof(RtpHeader);
                
                if (first_packet) {
                    expected_sequence = sequence;
                    first_packet = false;
                }
                
                // 检查序列号是否连续
                if (sequence == expected_sequence) {
                    // 将负载数据添加到当前帧
                    current_frame.insert(current_frame.end(), 
                                        buffer + sizeof(RtpHeader), 
                                        buffer + received_size);
                    
                    // 更新期望的序列号
                    expected_sequence++;
                    
                    // 检查是否收到了完整的一帧
                    if (current_frame.size() >= frame_size) {
                        frame_count++;
                        LOG_INFO("Received frame " + std::to_string(frame_count));
                        
                        // 写入输出文件
                        output_stream.write(reinterpret_cast<const char*>(current_frame.data()), frame_size);
                        
                        // 清空当前帧缓冲区
                        current_frame.clear();
                    }
                } else {
                    LOG_WARNING("Received out-of-order packet. Expected: " + 
                               std::to_string(expected_sequence) + ", Got: " + 
                               std::to_string(sequence));
                    // 可以在这里添加更复杂的丢包处理逻辑
                }
            }
            
            // 短暂延迟，避免CPU占用过高
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        // 清理
        output_stream.close();
        rtp_transport.close();
        
        LOG_INFO("RTP YUV receiver completed successfully. Received " + std::to_string(frame_count) + " frames.");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error in RTP YUV receiver: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}

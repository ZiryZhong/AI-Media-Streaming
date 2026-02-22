#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstring>
#include "rtp_transport.h"
#include "logger.h"

// 测试RTP YUV发送功能
int main(int argc, char* argv[]) {
    LOG_INFO("Starting RTP YUV sender");
    
    try {
        // 检查参数
        if (argc != 5) {
            LOG_ERROR("Usage: rtp_yuv_sender <yuv_file> <width> <height> <fps>");
            return 1;
        }
        
        std::string yuv_file = argv[1];
        int width = std::stoi(argv[2]);
        int height = std::stoi(argv[3]);
        int fps = std::stoi(argv[4]);
        
        // 计算一帧的大小（YUV420P格式）
        size_t frame_size = width * height * 3 / 2;
        LOG_INFO("Frame size: " + std::to_string(frame_size) + " bytes");
        
        // 打开YUV文件
        std::ifstream yuv_stream(yuv_file, std::ios::binary);
        if (!yuv_stream) {
            LOG_ERROR("Failed to open YUV file: " + yuv_file);
            return 1;
        }
        
        // 创建RTP传输实例
        RtpTransport rtp_transport;
        
        // 初始化RTP传输
        rtp_transport.initialize("127.0.0.1", 8888);
        
        // 设置负载类型为H.264（这里我们使用H.264的负载类型，但实际上传输的是YUV数据）
        rtp_transport.setPayloadType(96);
        
        // 分配帧缓冲区
        uint8_t* frame_buffer = new uint8_t[frame_size];
        
        // 数据包大小（1000字节）
        const size_t packet_size = 1000;
        
        int frame_count = 0;
        
        // 读取并发送每一帧
        while (yuv_stream.read(reinterpret_cast<char*>(frame_buffer), frame_size)) {
            frame_count++;
            LOG_INFO("Sending frame " + std::to_string(frame_count));
            
            // 切片发送
            size_t offset = 0;
            while (offset < frame_size) {
                // 计算当前包的大小
                size_t current_packet_size = std::min(packet_size, frame_size - offset);
                
                // 发送RTP包
                rtp_transport.sendRtpPacket(frame_buffer + offset, current_packet_size);
                
                // 更新偏移量
                offset += current_packet_size;
                
                // 短暂延迟，模拟实际网络传输
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            
            // 发送RTCP发送报告
            if (frame_count % 5 == 0) {
                LOG_INFO("Sending RTCP SR packet");
                uint8_t rtcp_data[20];
                memset(rtcp_data, 0, 20);
                rtp_transport.sendRtcpPacket(RtcpPacketType::SR, rtcp_data, 20);
            }
            
            // 按照指定的帧率等待
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
        }
        
        // 发送RTCP再见包
        LOG_INFO("Sending RTCP BYE packet");
        uint8_t bye_data[8];
        memset(bye_data, 0, 8);
        rtp_transport.sendRtcpPacket(RtcpPacketType::BYE, bye_data, 8);
        
        // 清理
        delete[] frame_buffer;
        yuv_stream.close();
        rtp_transport.close();
        
        LOG_INFO("RTP YUV sender completed successfully. Sent " + std::to_string(frame_count) + " frames.");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error in RTP YUV sender: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}

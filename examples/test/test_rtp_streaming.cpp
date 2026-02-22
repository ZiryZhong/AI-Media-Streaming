#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include "rtp_transport.h"
#include "logger.h"

// 测试RTP传输功能
int main() {
    LOG_INFO("Starting RTP streaming test");
    
    try {
        // 创建RTP传输实例
        RtpTransport rtp_transport;
        
        // 初始化RTP传输
        rtp_transport.initialize("127.0.0.1", 8888);
        
        // 设置负载类型为H.264
        rtp_transport.setPayloadType(96);
        
        // 模拟视频数据
        const size_t frame_size = 1024;
        uint8_t frame_data[frame_size];
        memset(frame_data, 0xAA, frame_size);
        
        // 发送10个RTP包
        for (int i = 0; i < 10; ++i) {
            // 修改帧数据以模拟不同的视频帧
            frame_data[0] = static_cast<uint8_t>(i);
            
            LOG_INFO("Sending RTP packet " + std::to_string(i + 1) + "/10");
            rtp_transport.sendRtpPacket(frame_data, frame_size);
            
            // 发送RTCP发送报告
            if (i % 3 == 0) {
                LOG_INFO("Sending RTCP SR packet");
                uint8_t rtcp_data[20];
                memset(rtcp_data, 0, 20);
                rtp_transport.sendRtcpPacket(RtcpPacketType::SR, rtcp_data, 20);
            }
            
            // 等待一段时间
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 发送RTCP再见包
        LOG_INFO("Sending RTCP BYE packet");
        uint8_t bye_data[8];
        memset(bye_data, 0, 8);
        rtp_transport.sendRtcpPacket(RtcpPacketType::BYE, bye_data, 8);
        
        // 关闭传输
        rtp_transport.close();
        LOG_INFO("RTP streaming test completed successfully");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error in RTP streaming test: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}

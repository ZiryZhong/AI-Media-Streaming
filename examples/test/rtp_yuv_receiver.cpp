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
        rtp_transport.initialize("0.0.0.0", 8889); // 使用0.0.0.0绑定到所有接口
        
        // 设置负载类型为H.264
        rtp_transport.setPayloadType(96);
        
        // 用于存储接收到的数据包
        std::vector<uint8_t> current_frame;
        current_frame.reserve(frame_size);
        
        // 用于跟踪数据包的序列号
        uint16_t expected_sequence = 0;
        bool first_packet = true;
        int consecutive_losses = 0; // 连续丢包计数
        int max_consecutive_losses = 10; // 最大连续丢包数，超过则请求关键帧
        
        int frame_count = 0;
        bool running = true;
        
        // RTCP统计信息
        uint32_t received_packets = 0;
        uint32_t lost_packets = 0;
        uint32_t last_sequence = 0;
        
        // 接收数据包
        int rtcp_interval_counter = 0; // 用于定期发送RTCP包的计数器
        while (running && frame_count < max_frames) {
            // 分配缓冲区
            uint8_t buffer[RTP_MAX_PACKET_SIZE];
            size_t received_size = 0;
            
            // 接收RTP包
            rtp_transport.receiveRtpPacket(buffer, RTP_MAX_PACKET_SIZE, received_size);
            
            LOG_DEBUG("Received RTP packet size: " + std::to_string(received_size) + " bytes");
            
            if (received_size > 0) {
                // 解析RTP头部
                RtpHeader* header = reinterpret_cast<RtpHeader*>(buffer);
                uint16_t sequence = ntohs(header->sequence);
                
                // 计算负载大小
                size_t payload_size = received_size - sizeof(RtpHeader);
                
                LOG_DEBUG("Received RTP packet - Seq: " + std::to_string(sequence) + ", Payload size: " + std::to_string(payload_size) + " bytes");
                
                if (first_packet) {
                    expected_sequence = sequence;
                    first_packet = false;
                    LOG_INFO("Received first packet with sequence: " + std::to_string(sequence));
                }
                
                // 更新统计信息
                received_packets++;
                LOG_INFO("Total received packets: " + std::to_string(received_packets));
                
                if (received_packets > 1) {
                    uint16_t expected = last_sequence + 1;
                    if (sequence > expected) {
                        lost_packets += sequence - expected;
                        LOG_WARNING("Detected packet loss. Expected: " + std::to_string(expected) + ", Got: " + std::to_string(sequence) + ", Lost: " + std::to_string(sequence - expected) + " packets");
                    }
                }
                last_sequence = sequence;
                
                // 检查序列号是否连续
                if (sequence == expected_sequence) {
                    // 重置连续丢包计数
                    consecutive_losses = 0;
                    
                    // 将负载数据添加到当前帧
                    current_frame.insert(current_frame.end(), 
                                        buffer + sizeof(RtpHeader), 
                                        buffer + received_size);
                    
                    // 更新期望的序列号
                    expected_sequence++;
                    
                    // 检查是否收到了完整的一帧
                    LOG_DEBUG("Current frame size: " + std::to_string(current_frame.size()) + ", Frame size: " + std::to_string(frame_size));
                    if (current_frame.size() >= frame_size) {
                        frame_count++;
                        LOG_INFO("Received frame " + std::to_string(frame_count));
                        
                        // 写入输出文件
                        output_stream.write(reinterpret_cast<const char*>(current_frame.data()), frame_size);
                        
                        // 清空当前帧缓冲区
                        current_frame.clear();
                    }
                } else if (sequence > expected_sequence) {
                    // 检测到丢包
                    uint16_t lost_count = sequence - expected_sequence;
                    LOG_WARNING("Detected packet loss. Expected: " + 
                               std::to_string(expected_sequence) + ", Got: " + 
                               std::to_string(sequence) + ", Lost: " + 
                               std::to_string(lost_count) + " packets");
                    
                    // 发送NACK请求
                    rtp_transport.sendNackRequest(expected_sequence, lost_count);
                    
                    // 更新连续丢包计数
                    consecutive_losses += lost_count;
                    
                    // 检查是否需要请求关键帧
                    if (consecutive_losses >= max_consecutive_losses) {
                        LOG_WARNING("Too many consecutive losses, requesting key frame");
                        rtp_transport.sendKeyFrameRequest();
                        consecutive_losses = 0;
                        // 清空当前帧缓冲区，准备接收新的关键帧
                        current_frame.clear();
                    }
                    
                    // 更新期望的序列号
                    expected_sequence = sequence + 1;
                    
                    // 将当前数据包添加到帧中
                    current_frame.insert(current_frame.end(), 
                                        buffer + sizeof(RtpHeader), 
                                        buffer + received_size);
                } else {
                    // 收到重复的数据包
                    LOG_WARNING("Received duplicate packet. Expected: " + 
                               std::to_string(expected_sequence) + ", Got: " + 
                               std::to_string(sequence));
                }
                
                // 每接收5个数据包发送一次RTCP接收报告，不管是否连续
                if (received_packets > 0 && received_packets % 5 == 0) {
                    LOG_INFO("Sending RTCP RR packet after receiving " + std::to_string(received_packets) + " packets");
                    uint8_t rtcp_data[20];
                    memset(rtcp_data, 0, 20);
                    rtp_transport.sendRtcpPacket(RtcpPacketType::RR, rtcp_data, 20);
                    LOG_INFO("RTCP RR packet sent successfully");
                }
            }
            
            // 定期发送RTCP接收报告，每100ms发送一次
            rtcp_interval_counter++;
            if (rtcp_interval_counter >= 100) {
                if (received_packets > 0) {
                    LOG_INFO("Sending periodic RTCP RR packet");
                    uint8_t rtcp_data[20];
                    memset(rtcp_data, 0, 20);
                    rtp_transport.sendRtcpPacket(RtcpPacketType::RR, rtcp_data, 20);
                    LOG_INFO("Periodic RTCP RR packet sent successfully");
                }
                rtcp_interval_counter = 0;
            }
            
            // 检查是否有RTCP包需要处理
            uint8_t rtcp_buffer[1500];
            size_t rtcp_size = 0;
            rtp_transport.receiveRtcpPacket(rtcp_buffer, 1500, rtcp_size);
            
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

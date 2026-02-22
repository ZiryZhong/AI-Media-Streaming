#include "rtp_transport.h"
#include "logger.h"
#include "network_transport.h"
#include <random>
#include <cstring>
#include <memory>
#include <arpa/inet.h>

// 声明createNetworkTransport函数
std::shared_ptr<NetworkTransport> createNetworkTransport();

// 生成随机SSRC
uint32_t RtpTransport::generateSSRC() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);
    return dist(gen);
}

// 构造函数
RtpTransport::RtpTransport() : 
    sequence_number(0),
    timestamp(0),
    ssrc(generateSSRC()),
    payload_type(96) { // 默认使用H.264负载类型
    network_transport = createNetworkTransport();
    LOG_INFO("RTP transport initialized with SSRC: " + std::to_string(ssrc));
}

// 初始化RTP传输
void RtpTransport::initialize(const std::string& address, int port) {
    LOG_INFO("Initializing RTP transport: " + address + ":" + std::to_string(port));
    network_transport->initialize(address, port);
}

// 发送RTP包
void RtpTransport::sendRtpPacket(const uint8_t* data, size_t size) {
    LOG_DEBUG("Sending RTP packet with payload size: " + std::to_string(size) + " bytes");
    
    // 检查包大小
    if (size > RTP_MAX_PACKET_SIZE - sizeof(RtpHeader)) {
        LOG_WARNING("Payload size exceeds RTP maximum packet size, truncating");
        size = RTP_MAX_PACKET_SIZE - sizeof(RtpHeader);
    }
    
    // 分配缓冲区
    uint8_t buffer[RTP_MAX_PACKET_SIZE];
    RtpHeader* header = reinterpret_cast<RtpHeader*>(buffer);
    
    // 填充RTP头部
    header->version = 2;
    header->padding = 0;
    header->extension = 0;
    header->csrc_count = 0;
    header->marker = 0;
    header->payload_type = payload_type;
    header->sequence = htons(sequence_number++);
    header->timestamp = htonl(timestamp);
    header->ssrc = htonl(ssrc);
    
    // 复制负载数据
    memcpy(buffer + sizeof(RtpHeader), data, size);
    
    // 发送数据包
    network_transport->sendPacket(buffer, sizeof(RtpHeader) + size);
    
    // 更新时间戳（假设25fps）
    timestamp += 90000 / 25;
}

// 接收RTP包
void RtpTransport::receiveRtpPacket(uint8_t* buffer, size_t bufferSize, size_t& receivedSize) {
    LOG_DEBUG("Receiving RTP packet into buffer of size: " + std::to_string(bufferSize) + " bytes");
    
    // 接收数据包
    network_transport->receivePacket(buffer, bufferSize, receivedSize);
    
    if (receivedSize > 0) {
        // 解析RTP头部
        RtpHeader* header = reinterpret_cast<RtpHeader*>(buffer);
        uint16_t sequence = ntohs(header->sequence);
        uint32_t timestamp = ntohl(header->timestamp);
        uint32_t ssrc = ntohl(header->ssrc);
        
        LOG_DEBUG("Received RTP packet - Seq: " + std::to_string(sequence) + 
                 ", Timestamp: " + std::to_string(timestamp) + 
                 ", SSRC: " + std::to_string(ssrc) + 
                 ", Payload size: " + std::to_string(receivedSize - sizeof(RtpHeader)) + " bytes");
    }
}

// 发送RTCP包
void RtpTransport::sendRtcpPacket(RtcpPacketType type, const uint8_t* data, size_t size) {
    LOG_DEBUG("Sending RTCP packet of type: " + std::to_string(static_cast<uint8_t>(type)));
    
    // 分配缓冲区
    uint8_t buffer[1500];
    RtcpHeader* header = reinterpret_cast<RtcpHeader*>(buffer);
    
    // 填充RTCP头部
    header->version = 2;
    header->padding = 0;
    header->count = 0;
    header->packet_type = static_cast<uint8_t>(type);
    header->length = htons(static_cast<uint16_t>((size + sizeof(RtcpHeader)) / 4 - 1));
    
    // 复制数据
    if (data && size > 0) {
        memcpy(buffer + sizeof(RtcpHeader), data, size);
    }
    
    // 发送数据包
    network_transport->sendPacket(buffer, sizeof(RtcpHeader) + size);
}

// 接收RTCP包
void RtpTransport::receiveRtcpPacket(uint8_t* buffer, size_t bufferSize, size_t& receivedSize) {
    LOG_DEBUG("Receiving RTCP packet into buffer of size: " + std::to_string(bufferSize) + " bytes");
    
    // 接收数据包
    network_transport->receivePacket(buffer, bufferSize, receivedSize);
    
    if (receivedSize > 0) {
        // 解析RTCP头部
        RtcpHeader* header = reinterpret_cast<RtcpHeader*>(buffer);
        uint8_t packet_type = header->packet_type;
        uint16_t length = ntohs(header->length);
        
        LOG_DEBUG("Received RTCP packet - Type: " + std::to_string(packet_type) + 
                 ", Length: " + std::to_string(length));
    }
}

// 关闭传输
void RtpTransport::close() {
    LOG_INFO("Closing RTP transport");
    network_transport->close();
}

// 设置负载类型
void RtpTransport::setPayloadType(uint8_t type) {
    payload_type = type;
    LOG_INFO("Set RTP payload type to: " + std::to_string(type));
}

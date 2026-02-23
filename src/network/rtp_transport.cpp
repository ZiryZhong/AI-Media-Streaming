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
    payload_type(96), // 默认使用H.264负载类型
    max_cache_size(1000) { // 最多缓存1000个数据包
    network_transport = createNetworkTransport();
    LOG_INFO("RTP transport initialized with SSRC: " + std::to_string(ssrc));
}

// 初始化RTP传输
void RtpTransport::initialize(const std::string& address, int port) {
    LOG_INFO("Initializing RTP transport: " + address + ":" + std::to_string(port));
    network_transport->initialize(address, port);
}

// 发送RTP包
void RtpTransport::sendRtpPacket(const uint8_t* data, size_t size, bool is_key_frame) {
    LOG_DEBUG("Sending RTP packet with payload size: " + std::to_string(size) + " bytes, is_key_frame: " + std::to_string(is_key_frame));
    
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
    header->marker = is_key_frame ? 1 : 0; // 关键帧设置标记位
    header->payload_type = payload_type;
    header->sequence = htons(sequence_number);
    header->timestamp = htonl(timestamp);
    header->ssrc = htonl(ssrc);
    
    // 复制负载数据
    memcpy(buffer + sizeof(RtpHeader), data, size);
    
    // 发送数据包
    network_transport->sendPacket(buffer, sizeof(RtpHeader) + size);
    
    // 缓存数据包，用于重传
    PacketCacheItem cache_item;
    cache_item.sequence = sequence_number;
    cache_item.timestamp = timestamp;
    cache_item.data.resize(sizeof(RtpHeader) + size);
    memcpy(cache_item.data.data(), buffer, sizeof(RtpHeader) + size);
    cache_item.is_key_frame = is_key_frame;
    
    packet_cache[sequence_number] = cache_item;
    
    // 限制缓存大小
    if (packet_cache.size() > max_cache_size) {
        // 删除最旧的数据包
        auto oldest = packet_cache.begin();
        packet_cache.erase(oldest);
    }
    
    // 更新序列号和时间戳
    sequence_number++;
    timestamp += 90000 / 25; // 假设25fps
}

// 处理NACK请求
void RtpTransport::handleNackRequest(const NackPacket& nack) {
    LOG_INFO("Handling NACK request: first_seq=" + std::to_string(nack.first_seq) + ", lost_packets=" + std::to_string(nack.lost_packets));
    
    // 重传丢失的数据包
    for (uint16_t i = 0; i < nack.lost_packets; i++) {
        uint16_t seq = nack.first_seq + i;
        
        // 检查数据包是否在缓存中
        auto it = packet_cache.find(seq);
        if (it != packet_cache.end()) {
            LOG_INFO("Retransmitting packet with sequence: " + std::to_string(seq));
            // 重传数据包
            network_transport->sendPacket(it->second.data.data(), it->second.data.size());
        } else {
            LOG_WARNING("Packet not found in cache for retransmission: " + std::to_string(seq));
        }
    }
}

// 处理关键帧请求
void RtpTransport::handleKeyFrameRequest(const KeyFrameRequest& request) {
    LOG_INFO("Handling key frame request from SSRC: " + std::to_string(request.ssrc));
    // 这里需要通知上层应用生成并发送关键帧
    // 实际实现中，应该通过回调或其他机制通知应用层
    LOG_INFO("Requesting key frame from application");
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
        
        // 处理不同类型的RTCP包
        if (packet_type == static_cast<uint8_t>(RtcpPacketType::NACK)) {
            // 处理NACK请求
            if (receivedSize >= sizeof(RtcpHeader) + sizeof(NackPacket)) {
                NackPacket* nack = reinterpret_cast<NackPacket*>(buffer + sizeof(RtcpHeader));
                handleNackRequest(*nack);
            }
        } else if (packet_type == static_cast<uint8_t>(RtcpPacketType::APP)) {
            // 处理APP包，用于关键帧请求
            if (receivedSize >= sizeof(RtcpHeader) + sizeof(KeyFrameRequest)) {
                KeyFrameRequest* request = reinterpret_cast<KeyFrameRequest*>(buffer + sizeof(RtcpHeader));
                if (request->request_type == 1) {
                    handleKeyFrameRequest(*request);
                }
            }
        } else if (packet_type == static_cast<uint8_t>(RtcpPacketType::RR)) {
            // 处理接收报告
            LOG_INFO("Received RTCP RR packet");
        } else if (packet_type == static_cast<uint8_t>(RtcpPacketType::SR)) {
            // 处理发送报告
            LOG_INFO("Received RTCP SR packet");
        }
    }
}

// 发送NACK请求
void RtpTransport::sendNackRequest(uint16_t first_seq, uint16_t lost_packets) {
    LOG_INFO("Sending NACK request: first_seq=" + std::to_string(first_seq) + ", lost_packets=" + std::to_string(lost_packets));
    
    // 分配缓冲区
    uint8_t buffer[sizeof(RtcpHeader) + sizeof(NackPacket)];
    
    // 填充RTCP头部
    RtcpHeader* header = reinterpret_cast<RtcpHeader*>(buffer);
    header->version = 2;
    header->padding = 0;
    header->count = 0;
    header->packet_type = static_cast<uint8_t>(RtcpPacketType::NACK);
    header->length = htons(sizeof(NackPacket) / 4); // RTCP长度单位是4字节
    
    // 填充NACK数据
    NackPacket* nack = reinterpret_cast<NackPacket*>(buffer + sizeof(RtcpHeader));
    nack->ssrc = htonl(ssrc);
    nack->lost_packets = htons(lost_packets);
    nack->first_seq = htons(first_seq);
    
    // 发送NACK请求
    network_transport->sendPacket(buffer, sizeof(buffer));
}

// 发送关键帧请求
void RtpTransport::sendKeyFrameRequest() {
    LOG_INFO("Sending key frame request");
    
    // 分配缓冲区
    uint8_t buffer[sizeof(RtcpHeader) + sizeof(KeyFrameRequest)];
    
    // 填充RTCP头部
    RtcpHeader* header = reinterpret_cast<RtcpHeader*>(buffer);
    header->version = 2;
    header->padding = 0;
    header->count = 0;
    header->packet_type = static_cast<uint8_t>(RtcpPacketType::APP);
    header->length = htons(sizeof(KeyFrameRequest) / 4); // RTCP长度单位是4字节
    
    // 填充关键帧请求数据
    KeyFrameRequest* request = reinterpret_cast<KeyFrameRequest*>(buffer + sizeof(RtcpHeader));
    request->ssrc = htonl(ssrc);
    request->request_type = 1; // 1表示关键帧请求
    
    // 发送关键帧请求
    network_transport->sendPacket(buffer, sizeof(buffer));
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

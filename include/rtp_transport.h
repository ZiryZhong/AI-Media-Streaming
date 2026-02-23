#ifndef RTP_TRANSPORT_H
#define RTP_TRANSPORT_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "network_transport.h"

// RTP包的最大大小
const size_t RTP_MAX_PACKET_SIZE = 1500;

// RTP头部结构
typedef struct {
    uint8_t version:2;     // 版本号，固定为2
    uint8_t padding:1;     // 填充标志
    uint8_t extension:1;   // 扩展标志
    uint8_t csrc_count:4;  // CSRC计数器
    uint8_t marker:1;       // 标记位
    uint8_t payload_type:7; // 负载类型
    uint16_t sequence;      // 序列号
    uint32_t timestamp;     // 时间戳
    uint32_t ssrc;          // 同步源标识符
    // CSRC标识符（可选）
} RtpHeader;

// RTCP包类型
enum class RtcpPacketType {
    SR = 200,    // 发送报告
    RR = 201,    // 接收报告
    SDES = 202,  // 源描述
    BYE = 203,   // 再见
    APP = 204,   // 应用特定
    NACK = 205   // 否定确认（用于请求重传）
};

// NACK包结构
typedef struct {
    uint32_t ssrc;          // 同步源标识符
    uint16_t lost_packets;  // 丢失的数据包数量
    uint16_t first_seq;      // 第一个丢失的数据包序列号
    // 后续可以添加更多丢失的序列号
} NackPacket;

// 关键帧请求结构
typedef struct {
    uint32_t ssrc;          // 同步源标识符
    uint8_t request_type;   // 请求类型：1表示关键帧请求
} KeyFrameRequest;

// 数据包缓存项
typedef struct {
    uint16_t sequence;      // 序列号
    uint32_t timestamp;     // 时间戳
    std::vector<uint8_t> data; // 数据包数据
    bool is_key_frame;      // 是否为关键帧
} PacketCacheItem;

// RTCP头部结构
typedef struct {
    uint8_t version:2;     // 版本号，固定为2
    uint8_t padding:1;     // 填充标志
    uint8_t count:5;       // 计数器
    uint8_t packet_type;    // 包类型
    uint16_t length;        // 长度
} RtcpHeader;

// RTP传输类
class RtpTransport {
private:
    std::shared_ptr<NetworkTransport> network_transport;
    uint16_t sequence_number;
    uint32_t timestamp;
    uint32_t ssrc;
    uint8_t payload_type;
    
    // 数据包缓存，用于重传
    std::map<uint16_t, PacketCacheItem> packet_cache;
    size_t max_cache_size; // 最大缓存大小
    
    // 生成随机SSRC
    uint32_t generateSSRC();
    
    // 处理NACK请求
    void handleNackRequest(const NackPacket& nack);
    
    // 处理关键帧请求
    void handleKeyFrameRequest(const KeyFrameRequest& request);
    
public:
    RtpTransport();
    ~RtpTransport() = default;
    
    // 初始化RTP传输
    void initialize(const std::string& address, int port);
    
    // 发送RTP包
    void sendRtpPacket(const uint8_t* data, size_t size, bool is_key_frame = false);
    
    // 接收RTP包
    void receiveRtpPacket(uint8_t* buffer, size_t bufferSize, size_t& receivedSize);
    
    // 发送RTCP包
    void sendRtcpPacket(RtcpPacketType type, const uint8_t* data, size_t size);
    
    // 接收RTCP包
    void receiveRtcpPacket(uint8_t* buffer, size_t bufferSize, size_t& receivedSize);
    
    // 发送NACK请求
    void sendNackRequest(uint16_t first_seq, uint16_t lost_packets);
    
    // 发送关键帧请求
    void sendKeyFrameRequest();
    
    // 关闭传输
    void close();
    
    // 设置负载类型
    void setPayloadType(uint8_t type);
};

#endif // RTP_TRANSPORT_H

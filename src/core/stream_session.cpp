#include "stream_session.h"
#include "rate_control_interface.h"
#include "codec_interface.h"
#include "network_transport.h"
#include "logger.h"
#include <string>

StreamSession::StreamSession(const std::string& id) : sessionId(id) {
    LOG_INFO("StreamSession created: " + sessionId);
}

StreamSession::~StreamSession() {
    LOG_INFO("StreamSession destroyed: " + sessionId);
}

void StreamSession::setRateControl(std::shared_ptr<RateControlInterface> rc) {
    rateControl = rc;
    LOG_INFO("Rate control set for session: " + sessionId);
}

void StreamSession::setCodec(std::shared_ptr<CodecInterface> c) {
    codec = c;
    LOG_INFO("Codec set for session: " + sessionId);
}

void StreamSession::setNetworkTransport(std::shared_ptr<NetworkTransport> nt) {
    network = nt;
    LOG_INFO("Network transport set for session: " + sessionId);
}

void StreamSession::startStreaming() {
    LOG_INFO("Started streaming for session: " + sessionId);
    // 启动流媒体传输
}

void StreamSession::stopStreaming() {
    LOG_INFO("Stopped streaming for session: " + sessionId);
    // 停止流媒体传输
}

void StreamSession::sendFrame(const uint8_t* data, size_t size) {
    if (!rateControl || !codec || !network) {
        LOG_ERROR("Missing components for streaming in session: " + sessionId);
        return;
    }

    LOG_DEBUG("Sending frame of size: " + std::to_string(size) + " bytes");

    // 获取目标码率
    int targetBitrate = rateControl->getTargetBitrate();
    LOG_DEBUG("Target bitrate: " + std::to_string(targetBitrate) + " bps");

    // 编码帧
    auto encodedData = codec->encode(data, size, targetBitrate);
    LOG_DEBUG("Encoded frame size: " + std::to_string(encodedData.size()) + " bytes");

    // 发送编码后的数据
    network->sendPacket(encodedData.data(), encodedData.size());
    LOG_DEBUG("Sent encoded frame over network");
}

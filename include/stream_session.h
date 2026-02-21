#ifndef STREAM_SESSION_H
#define STREAM_SESSION_H

#include <string>
#include <memory>
#include <cstdint>

class RateControlInterface;
class CodecInterface;
class NetworkTransport;

class StreamSession {
private:
    std::string sessionId;
    std::shared_ptr<RateControlInterface> rateControl;
    std::shared_ptr<CodecInterface> codec;
    std::shared_ptr<NetworkTransport> network;

public:
    StreamSession(const std::string& id);
    ~StreamSession();

    void setRateControl(std::shared_ptr<RateControlInterface> rc);
    void setCodec(std::shared_ptr<CodecInterface> c);
    void setNetworkTransport(std::shared_ptr<NetworkTransport> nt);

    void startStreaming();
    void stopStreaming();
    void sendFrame(const uint8_t* data, size_t size);
};

#endif // STREAM_SESSION_H

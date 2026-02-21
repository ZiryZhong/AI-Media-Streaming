#include "stream_manager.h"
#include "stream_session.h"
#include "rate_control_interface.h"
#include "codec_interface.h"
#include "network_transport.h"
#include <memory>

// 声明外部工厂函数
extern "C" RateControlInterface* createRateControlInstance();
extern "C" CodecInterface* createCodecInstance();
extern std::shared_ptr<NetworkTransport> createNetworkTransport();

int main() {
    // 创建流管理器
    StreamManager manager;

    // 创建流会话
    auto session = manager.createSession("test_session");

    // 创建并设置AI码率控制
    std::shared_ptr<RateControlInterface> rateControl(createRateControlInstance());
    session->setRateControl(rateControl);

    // 创建并设置自定义编解码器
    std::shared_ptr<CodecInterface> codec(createCodecInstance());
    session->setCodec(codec);

    // 创建并设置网络传输
    auto network = createNetworkTransport();
    session->setNetworkTransport(network);

    // 启动流媒体传输
    session->startStreaming();

    // 模拟发送一帧数据
    uint8_t testData[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    session->sendFrame(testData, sizeof(testData));

    // 停止流媒体传输
    session->stopStreaming();

    // 移除会话
    manager.removeSession("test_session");

    return 0;
}

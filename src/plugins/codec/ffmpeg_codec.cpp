#include "codec_interface.h"
#include "logger.h"
#include <string>
#include <vector>
#include <cstdint>

// FFmpeg headers
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

class FFmpegCodec : public CodecInterface {
private:
    AVCodec* encoder;
    AVCodecContext* encoder_ctx;
    AVFrame* frame;
    AVPacket* packet;
    int width;
    int height;

public:
    FFmpegCodec() : encoder(nullptr), encoder_ctx(nullptr), frame(nullptr), packet(nullptr), width(640), height(480) {
        LOG_INFO("FFmpegCodec initialized");
    }

    ~FFmpegCodec() {
        if (packet) {
            av_packet_free(&packet);
        }
        if (frame) {
            av_frame_free(&frame);
        }
        if (encoder_ctx) {
            avcodec_free_context(&encoder_ctx);
        }
        LOG_INFO("FFmpegCodec destroyed");
    }

    void initialize() override {
        LOG_INFO("Initializing FFmpeg codec");
        
        // 注册所有编解码器
        avcodec_register_all();
        
        // 查找H.264编码器
        encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!encoder) {
            LOG_ERROR("Failed to find H.264 encoder");
            return;
        }
        
        // 创建编码器上下文
        encoder_ctx = avcodec_alloc_context3(encoder);
        if (!encoder_ctx) {
            LOG_ERROR("Failed to allocate encoder context");
            return;
        }
        
        // 设置编码器参数
        encoder_ctx->bit_rate = 400000;
        encoder_ctx->width = width;
        encoder_ctx->height = height;
        encoder_ctx->time_base = {1, 25};
        encoder_ctx->framerate = {25, 1};
        encoder_ctx->gop_size = 10;
        encoder_ctx->max_b_frames = 1;
        encoder_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
        
        // 设置编码器选项
        av_opt_set(encoder_ctx->priv_data, "preset", "ultrafast", 0);
        av_opt_set(encoder_ctx->priv_data, "tune", "zerolatency", 0);
        
        // 打开编码器
        if (avcodec_open2(encoder_ctx, encoder, nullptr) < 0) {
            LOG_ERROR("Failed to open encoder");
            return;
        }
        
        // 分配帧和数据包
        frame = av_frame_alloc();
        if (!frame) {
            LOG_ERROR("Failed to allocate frame");
            return;
        }
        
        frame->format = encoder_ctx->pix_fmt;
        frame->width = encoder_ctx->width;
        frame->height = encoder_ctx->height;
        
        if (av_frame_get_buffer(frame, 0) < 0) {
            LOG_ERROR("Failed to allocate frame buffer");
            return;
        }
        
        packet = av_packet_alloc();
        if (!packet) {
            LOG_ERROR("Failed to allocate packet");
            return;
        }
        
        LOG_INFO("FFmpeg codec initialized successfully");
    }

    std::vector<uint8_t> encode(const uint8_t* data, size_t size, int targetBitrate) override {
        LOG_DEBUG("Encoding frame with target bitrate: " + std::to_string(targetBitrate) + " bps");
        
        if (!encoder_ctx || !frame || !packet) {
            LOG_ERROR("Codec not initialized");
            return {};
        }
        
        // 更新目标码率
        if (targetBitrate > 0) {
            encoder_ctx->bit_rate = targetBitrate;
        }
        
        // 填充帧数据（假设输入是YUV420P格式）
        int ret = av_frame_make_writable(frame);
        if (ret < 0) {
            LOG_ERROR("Failed to make frame writable");
            return {};
        }
        
        // 复制YUV数据到帧
        size_t y_size = width * height;
        size_t uv_size = y_size / 4;
        
        if (size >= y_size + 2 * uv_size) {
            memcpy(frame->data[0], data, y_size);           // Y分量
            memcpy(frame->data[1], data + y_size, uv_size);   // U分量
            memcpy(frame->data[2], data + y_size + uv_size, uv_size); // V分量
        } else {
            LOG_ERROR("Input data size too small for YUV420P format");
            return {};
        }
        
        // 编码帧
        ret = avcodec_send_frame(encoder_ctx, frame);
        if (ret < 0) {
            LOG_ERROR("Failed to send frame for encoding");
            return {};
        }
        
        std::vector<uint8_t> encodedData;
        
        // 接收编码后的数据包
        while (ret >= 0) {
            ret = avcodec_receive_packet(encoder_ctx, packet);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                LOG_ERROR("Failed to receive encoded packet");
                break;
            }
            
            // 复制编码数据
            encodedData.insert(encodedData.end(), packet->data, packet->data + packet->size);
            
            av_packet_unref(packet);
        }
        
        LOG_DEBUG("Encoded frame size: " + std::to_string(encodedData.size()) + " bytes");
        return encodedData;
    }

    std::vector<uint8_t> decode(const uint8_t* data, size_t size) override {
        LOG_DEBUG("Decoding frame of size: " + std::to_string(size) + " bytes");
        
        // 解码功能暂未实现
        std::vector<uint8_t> decodedData;
        return decodedData;
    }

    void setParameters(const char* key, const char* value) override {
        LOG_INFO("Setting codec parameter: " + std::string(key) + " = " + std::string(value));
        
        if (strcmp(key, "width") == 0) {
            width = std::stoi(value);
        } else if (strcmp(key, "height") == 0) {
            height = std::stoi(value);
        }
    }
};

// 创建FFmpeg编解码器实例的工厂方法
extern "C" CodecInterface* createFFmpegCodecInstance() {
    return new FFmpegCodec();
}

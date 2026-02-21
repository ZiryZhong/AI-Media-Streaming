#ifndef CODEC_INTERFACE_H
#define CODEC_INTERFACE_H

#include <vector>
#include <cstdint>

class CodecInterface {
public:
    virtual ~CodecInterface() = default;

    virtual void initialize() = 0;
    virtual std::vector<uint8_t> encode(const uint8_t* data, size_t size, int targetBitrate) = 0;
    virtual std::vector<uint8_t> decode(const uint8_t* data, size_t size) = 0;
    virtual void setParameters(const char* key, const char* value) = 0;
};

#endif // CODEC_INTERFACE_H

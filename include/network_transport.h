#ifndef NETWORK_TRANSPORT_H
#define NETWORK_TRANSPORT_H

#include <string>
#include <cstdint>

class NetworkTransport {
public:
    virtual ~NetworkTransport() = default;

    virtual void initialize(const std::string& address, int port) = 0;
    virtual void sendPacket(const uint8_t* data, size_t size) = 0;
    virtual void receivePacket(uint8_t* buffer, size_t bufferSize, size_t& receivedSize) = 0;
    virtual void close() = 0;
};

#endif // NETWORK_TRANSPORT_H

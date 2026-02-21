#ifndef RATE_CONTROL_INTERFACE_H
#define RATE_CONTROL_INTERFACE_H

class RateControlInterface {
public:
    virtual ~RateControlInterface() = default;

    virtual void initialize() = 0;
    virtual void updateNetworkConditions(float bandwidth, float packetLoss) = 0;
    virtual int getTargetBitrate() = 0;
    virtual void feedback(float quality, int actualBitrate) = 0;
};

#endif // RATE_CONTROL_INTERFACE_H

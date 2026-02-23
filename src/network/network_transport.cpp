#include "network_transport.h"
#include "logger.h"
#include <memory>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <errno.h>

// 网络传输层的实现
class UdpTransport : public NetworkTransport {
private:
    int socket_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in peer_addr; // 存储对等方的地址
    bool peer_addr_set; // 标记对等方地址是否已设置

public:
    UdpTransport() : socket_fd(-1), peer_addr_set(false) {
    }

    void initialize(const std::string& address, int port) override {
        LOG_INFO("Initializing UDP transport: " + address + ":" + std::to_string(port));
        
        // 创建UDP socket
        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd < 0) {
            LOG_ERROR("Failed to create socket");
            return;
        }
        
        // 设置socket为非阻塞模式
        int flags = fcntl(socket_fd, F_GETFL, 0);
        if (flags < 0) {
            LOG_ERROR("Failed to get socket flags");
            return;
        }
        if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
            LOG_ERROR("Failed to set socket to non-blocking mode");
            return;
        }
        
        // 设置服务器地址
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(address.c_str());
        
        // 对于发送端，不需要绑定端口，让系统自动分配
        // 对于接收端，需要绑定端口，以便接收数据包
        // 这里我们通过检查地址是否为0.0.0.0来决定是否绑定端口
        if (strcmp(address.c_str(), "0.0.0.0") == 0) {
            // 绑定端口，以便接收数据包
            int bind_result = bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
            if (bind_result < 0) {
                LOG_ERROR("Failed to bind socket to port " + std::to_string(port));
                return;
            }
            LOG_INFO("UDP socket initialized successfully and bound to port " + std::to_string(port));
        } else {
            // 不绑定端口，让系统自动分配
            LOG_INFO("UDP socket initialized successfully (no bind, using system-assigned port)");
        }
    }

    void sendPacket(const uint8_t* data, size_t size) override {
        LOG_DEBUG("Sending UDP packet of size: " + std::to_string(size) + " bytes");
        
        if (socket_fd < 0) {
            LOG_ERROR("Socket not initialized");
            return;
        }
        
        struct sockaddr_in* target_addr;
        socklen_t target_addr_len;
        
        // 如果已设置对等方地址，则使用对等方地址，否则使用初始化时设置的服务器地址
        if (peer_addr_set) {
            target_addr = &peer_addr;
            target_addr_len = sizeof(peer_addr);
        } else {
            target_addr = &server_addr;
            target_addr_len = sizeof(server_addr);
        }
        
        // 发送UDP数据包
        ssize_t sent_bytes = sendto(socket_fd, data, size, 0, 
                                   (struct sockaddr*)target_addr, target_addr_len);
        
        if (sent_bytes < 0) {
            LOG_ERROR("Failed to send packet");
        } else {
            LOG_DEBUG("Sent " + std::to_string(sent_bytes) + " bytes");
        }
    }

    void receivePacket(uint8_t* buffer, size_t bufferSize, size_t& receivedSize) override {
        LOG_DEBUG("Receiving UDP packet into buffer of size: " + std::to_string(bufferSize) + " bytes");
        
        if (socket_fd < 0) {
            LOG_ERROR("Socket not initialized");
            receivedSize = 0;
            return;
        }
        
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        
        // 接收UDP数据包
        ssize_t recv_bytes = recvfrom(socket_fd, buffer, bufferSize, 0, 
                                     (struct sockaddr*)&client_addr, &client_addr_len);
        
        if (recv_bytes < 0) {
            // 检查是否为非阻塞模式下的EAGAIN错误
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 没有数据可用，这是正常情况
                receivedSize = 0;
            } else {
                LOG_ERROR("Failed to receive packet: " + std::string(strerror(errno)));
                receivedSize = 0;
            }
        } else {
            receivedSize = static_cast<size_t>(recv_bytes);
            LOG_DEBUG("Received " + std::to_string(receivedSize) + " bytes");
            
            // 记录发送端的地址和端口
            peer_addr = client_addr;
            peer_addr_set = true;
        }
    }

    void close() override {
        LOG_INFO("Closing UDP transport");
        
        if (socket_fd >= 0) {
            ::close(socket_fd);
            socket_fd = -1;
        }
    }
};

// 创建网络传输实例的工厂方法
std::shared_ptr<NetworkTransport> createNetworkTransport() {
    return std::make_shared<UdpTransport>();
}

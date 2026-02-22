#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];
    
    // 创建UDP socket
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    
    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // 绑定socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(server_fd);
        return 1;
    }
    
    std::cout << "UDP server started on port 8888" << std::endl;
    
    // 接收数据包
    while (true) {
        ssize_t recv_bytes = recvfrom(server_fd, buffer, sizeof(buffer), 0, 
                                     (struct sockaddr*)&client_addr, &client_addr_len);
        
        if (recv_bytes < 0) {
            std::cerr << "Failed to receive packet" << std::endl;
            continue;
        }
        
        std::cout << "Received packet from " << inet_ntoa(client_addr.sin_addr) 
                  << ":" << ntohs(client_addr.sin_port) << std::endl;
        std::cout << "Packet size: " << recv_bytes << " bytes" << std::endl;
        std::cout << "Packet data: ";
        for (ssize_t i = 0; i < recv_bytes; i++) {
            std::cout << static_cast<int>(buffer[i]) << " ";
        }
        std::cout << std::endl;
    }
    
    // 关闭socket
    close(server_fd);
    
    return 0;
}

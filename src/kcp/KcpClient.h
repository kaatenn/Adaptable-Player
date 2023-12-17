//
// Created by 86137 on 2023/12/17.
//

#ifndef ADAPTABLE_UPLOADER_KCPCLIENT_H
#define ADAPTABLE_UPLOADER_KCPCLIENT_H

#include "ikcp.h"
#include "asio/asio.hpp"

#include <string>
#include <chrono>
#include "iostream"

class KCPClient {
public:
    KCPClient(asio::io_context& io_context, const std::string& server_ip, unsigned short server_port)
            : io_context(io_context), socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)),
              server_endpoint(asio::ip::address::from_string(server_ip), server_port) {

        kcp = ikcp_create(0x11223344, (void*)this);
        ikcp_setoutput(kcp, &KCPClient::udp_output);
    }

    ~KCPClient() {
        ikcp_release(kcp);
    }

    void update() {
        ikcp_update(kcp, iclock());
        // 其他处理，例如接收数据等
    }

    void send(const char *data, size_t length) {
        ikcp_send(kcp, data, length);
        std::cout << "send data: " << data << std::endl;
    }

private:
    asio::io_context& io_context;
    asio::ip::udp::socket socket;
    asio::ip::udp::endpoint server_endpoint;
    ikcpcb *kcp;

    static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
        KCPClient *client = (KCPClient*)user;
        client->socket.send_to(asio::buffer(buf, len), client->server_endpoint);
        return 0;
    }

    static inline uint32_t iclock()
    {
        using namespace std::chrono;
        return static_cast<uint32_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    }

};



#endif //ADAPTABLE_UPLOADER_KCPCLIENT_H

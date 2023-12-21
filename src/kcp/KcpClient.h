//
// Created by 86137 on 2023/12/17.
//

#ifndef ADAPTABLE_UPLOADER_KCPCLIENT_H
#define ADAPTABLE_UPLOADER_KCPCLIENT_H

#include "ikcp.h"

#include <string>
#include <chrono>
#include "iostream"
#include "fstream"

#include <asio.hpp>

using std::ofstream, std::ios;

class KCPClient {
public:
    bool should_exit = false;

    KCPClient(asio::io_context& io_context, const std::string& server_ip, unsigned short server_port);

    ~KCPClient() {
        ikcp_release(kcp);
    }

    void update();

    void send(const char *data, size_t length) {
        ikcp_send(kcp, data, length);
    }

    void start_receive();
    void on_receive(const char *data, size_t length);

private:
    asio::io_context& io_context;
    asio::ip::udp::socket socket;
    asio::ip::udp::endpoint server_endpoint;
    asio::steady_timer *timer;
    std::array<char, 1024> receive_buffer{};
    std::array<char, 1024> file_buffer{};
    ofstream fout;
    ikcpcb *kcp;

    static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user);

    static inline uint32_t iclock()
    {
        using namespace std::chrono;
        return static_cast<uint32_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    }
};



#endif //ADAPTABLE_UPLOADER_KCPCLIENT_H

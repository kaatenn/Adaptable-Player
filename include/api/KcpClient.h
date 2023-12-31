//
// Created by 86137 on 2023/12/17.
//

#ifndef ADAPTABLE_UPLOADER_KCPCLIENT_H
#define ADAPTABLE_UPLOADER_KCPCLIENT_H

#include "ikcp.h"
#include "detail/DataWrapper.hpp"
// TODO: Add DataWrapper

#include <string>
#include <chrono>
#include "map"
#include "vector"
#include "iostream"
#include "fstream"

#include "asio.hpp"
#include "ApplicationProtocolBase.h"

using std::ofstream, std::ios, std::vector, std::string, std::map;
using kaatenn::ApplicationProtocolBase;

class KCPClient {
public:
    bool should_exit = false;

    KCPClient(const std::string& server_ip, unsigned short server_port, DataWrapper*
    data_wrapper, IUINT32 kcp_conv, ApplicationProtocolBase* application_protocol);

    ~KCPClient() {
        ikcp_release(kcp);
    }

    void update();

    void send(const char *data, size_t length);

    void run_client();


private:
    asio::io_context io_context;
    asio::ip::udp::socket socket;
    asio::ip::udp::endpoint server_endpoint;
    asio::steady_timer *timer;
    std::array<char, 1024> receive_buffer{};
    std::array<char, 1024> file_buffer{};
    ofstream fout;
    ikcpcb *kcp;

    IUINT32 kcp_conv;

    DataWrapper* data_wrapper;
    ApplicationProtocolBase* application_protocol;
    std::thread asio_thread;

    void start_receive();
    void on_receive(const char *data, size_t length);

    static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user);

    static inline uint32_t iclock()
    {
        using namespace std::chrono;
        return static_cast<uint32_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    }
};



#endif //ADAPTABLE_UPLOADER_KCPCLIENT_H

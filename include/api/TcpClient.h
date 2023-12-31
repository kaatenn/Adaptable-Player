//
// Created by 86137 on 2023/12/28.
//

#ifndef ADAPTABLE_UPLOADER_TCPCLIENT_H
#define ADAPTABLE_UPLOADER_TCPCLIENT_H

#include "detail/DataWrapper.hpp"
#include "fstream"
#include "ApplicationProtocolBase.h"

#include "asio.hpp"

using namespace kaatenn;
class TCPClient {
public:
    TCPClient(const std::string& server_ip, unsigned short server_port, DataWrapper*
    data_wrapper, ApplicationProtocolBase* application_protocol);

    void send(const char* data, size_t length);

    void run_client();
private:
    asio::io_context io_context;
    asio::ip::tcp::socket socket;
    asio::ip::tcp::resolver resolver;
    asio::steady_timer *timer;
    std::array<char, 1024> receive_buffer{};
    DataWrapper* data_wrapper;

    std::thread asio_thread;

    ApplicationProtocolBase* application_protocol;

    bool should_exit = false;

    void do_receive();
    void start_receive();

    void update();
};


#endif //ADAPTABLE_UPLOADER_TCPCLIENT_H

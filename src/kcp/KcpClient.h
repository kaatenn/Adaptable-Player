//
// Created by 86137 on 2023/12/17.
//

#ifndef ADAPTABLE_UPLOADER_KCPCLIENT_H
#define ADAPTABLE_UPLOADER_KCPCLIENT_H

#include "ikcp.h"
#include "Data/DataWrapper.hpp"
// TODO: Add DataWrapper

#include <string>
#include <chrono>
#include "map"
#include "vector"
#include "iostream"
#include "fstream"
#include "Connection.hpp"

#include <asio.hpp>

using std::ofstream, std::ios, std::vector, std::string, std::map;

class KCPClient {
public:
    bool should_exit = false;

    KCPClient(asio::io_context& io_context, const std::string& server_ip, unsigned short server_port, DataWrapper*
    data_wrapper);

    ~KCPClient() {
        ikcp_release(kcp);
    }

    void update();

    void send(const string& url, const char *data, size_t length);

    void start_receive();
    void on_receive(const char *data, size_t length);

private:
    asio::io_context& io_context;
    asio::ip::udp::socket socket;
    asio::ip::udp::endpoint server_endpoint;
    asio::steady_timer *timer;
    std::array<char, 1024> receive_buffer{};
    std::array<char, 1024> file_buffer{};
    void check_need_file(const Connection& connection);
    ofstream fout;
    ikcpcb *kcp;

    string waiting_file_name;
    string ending_asserting_string;
    string waiting_url;
    int waiting_file_size = 0;

    DataWrapper* data_wrapper;

    static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user);

    static inline uint32_t iclock()
    {
        using namespace std::chrono;
        return static_cast<uint32_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    }
};



#endif //ADAPTABLE_UPLOADER_KCPCLIENT_H

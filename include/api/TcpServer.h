//
// Created by 86137 on 2023/12/28.
//

#ifndef ADAPTABLE_SERVER_TCPSERVER_H
#define ADAPTABLE_SERVER_TCPSERVER_H

#include "asio.hpp"
#include "string"
#include "ApplicationProtocolBase.h"

using std::string;
using kaatenn::ApplicationProtocolBase;

namespace kaatenn {
    class TCPServer {
    public:
        explicit TCPServer(unsigned short port, ApplicationProtocolBase* application_protocol);

        void start_receive();

        void run_server();

        ApplicationProtocolBase* application_protocol;

    private:
        asio::io_context io_context;
        asio::ip::tcp::socket socket;
        asio::ip::tcp::acceptor acceptor;
        asio::ip::tcp::endpoint remote_endpoint;
        asio::ip::tcp::endpoint endpoint;

        std::thread asio_thread;
    };


    class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
    public:
        explicit TCPConnection(asio::ip::tcp::socket socket, ApplicationProtocolBase* application_protocol) : socket
        (std::move(socket)), application_protocol(application_protocol) {};

        void start();

    private:
        ApplicationProtocolBase* application_protocol;

        void do_read();

        void on_receive(char *data, size_t recvd);

        void send(const char *data, size_t length);

        asio::ip::tcp::socket socket;
        std::array<char, 1024> receive_buffer;
        string ending_assert_string;
    };

}

#endif //ADAPTABLE_SERVER_TCPSERVER_H

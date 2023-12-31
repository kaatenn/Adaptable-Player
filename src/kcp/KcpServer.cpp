//
// Created by 86137 on 2023/12/17.
//

#include "api/KcpServer.h"
#include "fstream"
#include "string"
#include "detail/error_handler.h"


using std::string;
using namespace kaatenn;

KCPServer::KCPServer(unsigned short port, ApplicationProtocolBase *application_protocol, IUINT32 kcp_conv)
        : socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
          application_protocol(application_protocol), kcp_conv(kcp_conv) {
    check_port(port);

    // init kcp
    kcp = ikcp_create(kcp_conv, (void *) this);
    ikcp_setoutput(kcp, &KCPServer::udp_output);

    // add timer
    this->timer = new asio::steady_timer(io_context, asio::chrono::milliseconds(10));
    timer->async_wait([this](std::error_code ec) {
        if (!ec) {
            this->update();
        }
    });
    start_receive();

    asio_thread = std::thread([this]() { io_context.run(); });
}

void KCPServer::run_server() {
    asio_thread.join();
}

void KCPServer::start_receive() {
    socket.async_receive_from(
            asio::buffer(receive_buffer), remote_endpoint,
            [this](std::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    on_receive(receive_buffer.data(), bytes_recvd);
                } else {
                    std::cout << "receive error: " << ec.message() << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
    );
}

void KCPServer::on_receive(const char *data, size_t length) {
    ikcp_input(kcp, data, length); // manage a receiving package
    int recv_size;
    string res;
    while ((recv_size = ikcp_recv(kcp, file_buffer.data(), file_buffer.size())) > 0) {
        // When we could successfully receive a legal json, we assert the ending of the response.
        if (application_protocol->process_segment(file_buffer.data(), recv_size)) {
            res = application_protocol->get_response();
            this->send(res.data(), res.size());
            application_protocol->reset();
        }
    }
    socket.connect(remote_endpoint);
    start_receive();
}

void KCPServer::update() {
    ikcp_update(kcp, iclock());

    // refresh timer
    timer->expires_after(asio::chrono::milliseconds(10));
    timer->async_wait([this](std::error_code ec) {
        if (!ec) {
            this->update();
        }
    });
}

int KCPServer::udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
    auto *server = (KCPServer *) user;
    server->socket.async_send(asio::buffer(buf, len), [](std::error_code ec, std::size_t bytes_sent) {
        if (ec) {
            std::cout << "send error: " << ec.message() << std::endl;
        }
        if (!ec && bytes_sent > 0) {
            // std::cout << "send data: " << bytes_sent << std::endl;
        }
    });
    return 0;
}

void KCPServer::send(const char *data, size_t length) {
    int segment_size = 900;
    while (length > 0) {
        int sending_size = std::min(segment_size, (int) length);
        ikcp_send(kcp, data, sending_size);
        data += sending_size;
        length -= sending_size;
    }
}

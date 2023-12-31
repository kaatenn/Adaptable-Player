//
// Created by 86137 on 2023/12/17.
//

#include "api/KcpClient.h"

KCPClient::KCPClient(const std::string &server_ip, unsigned short server_port,
                     DataWrapper *data_wrapper, IUINT32 kcp_conv, ApplicationProtocolBase *application_protocol)
        : socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)),
          server_endpoint(asio::ip::address::from_string(server_ip), server_port), data_wrapper(data_wrapper)
          , kcp_conv(kcp_conv), application_protocol(application_protocol)
          {
    start_receive();
    kcp = ikcp_create(this->kcp_conv, (void *) this);
    socket.connect(server_endpoint);
    ikcp_setoutput(kcp, &KCPClient::udp_output);

    this->timer = new asio::steady_timer(io_context, asio::chrono::milliseconds(10));
    timer->async_wait([this](std::error_code ec) {
        if (!ec) {
            this->update();
        }
    });
    ikcp_nodelay(kcp, 1, 10, 2, 1);
    asio_thread = std::thread([this]() { io_context.run(); });
}

void KCPClient::start_receive() {
    socket.async_receive_from(
            asio::buffer(receive_buffer), server_endpoint,
            [this](std::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    on_receive(receive_buffer.data(), bytes_recvd); // unlike tcp, kcp may slice the buffer, so we
                    // need to manage the buffer in on_receive
                } else {
                    std::cout << "receive error: " << ec.message() << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
    );
}

void KCPClient::on_receive(const char *data, size_t length) {
    ikcp_input(kcp, receive_buffer.data(), length);
    // MANAGE ONLY ONE PACKET !!!

    int recv_size;

    while ((recv_size = ikcp_recv(kcp, file_buffer.data(), file_buffer.size())) > 0) {
        if (application_protocol->process_segment(file_buffer.data(), recv_size)) {
            string res = application_protocol->get_response();
            this->send(res.data(), res.size());\
            application_protocol->reset();
        }
    }

    start_receive();
}

int KCPClient::udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
    auto *client = (KCPClient *) user;
    client->socket.async_send_to(asio::buffer(buf, len), client->server_endpoint,
                                 [](std::error_code ec, std::size_t bytes_sent) {
                                     if (ec) {
                                         std::cout << "send error: " << ec.message() << std::endl;
                                     }
                                     if (!ec && bytes_sent > 0) {
                                         std::cout << "send data: " << bytes_sent << std::endl;
                                     }
                                 });
    return 0;
}

void KCPClient::update() {
    should_exit = data_wrapper->should_close.get_and_reset() != std::nullopt;
    if (should_exit) {
        io_context.stop();
        data_wrapper->should_close.set(true); // We do not know either the other client has closed the connection.
        return;
    }
    ikcp_update(kcp, iclock());

    // read request from kcp_send_queue
    optional<string> request;
    int segment_size = 900; // leave some space for kcp header and '\0'
    while ((request = data_wrapper->kcp_send_queue.try_pop()) != nullopt) {
        vector<char> request_buffer;
        request_buffer.resize(request.value().size());
        request_buffer.assign(request->begin(), request->end());
        // we don't need to slice buffer like tcp, kcp will do it for us
        for (int i = 0; i < request_buffer.size(); i += segment_size) {
            int size = std::min(segment_size, (int) request_buffer.size() - i);
            ikcp_send(kcp, request_buffer.data() + i, size);
        }
    }

    // refresh timer
    timer->expires_after(asio::chrono::milliseconds(10));
    timer->async_wait([this](std::error_code ec) {
        if (!ec) {
            this->update();
        }
    });
}

void KCPClient::send(const char *data, size_t length) {
    ikcp_send(kcp, data, length);
}

void KCPClient::run_client() {
    asio_thread.join();
}




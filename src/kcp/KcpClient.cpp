//
// Created by 86137 on 2023/12/17.
//

#include "KcpClient.h"

KCPClient::KCPClient(asio::io_context &io_context, const std::string &server_ip, unsigned short server_port,
                     DataWrapper* data_wrapper)
        : io_context(io_context), socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)),
          server_endpoint(asio::ip::address::from_string(server_ip), server_port), data_wrapper(data_wrapper) {

    kcp = ikcp_create(0x11223344, (void*)this);
    socket.connect(server_endpoint);
    ikcp_setoutput(kcp, &KCPClient::udp_output);

    this->timer = new asio::steady_timer(io_context, asio::chrono::milliseconds(10));
    timer->async_wait([this](std::error_code ec) {
        if (!ec) {
            this->update();
        }
    });
    ikcp_nodelay(kcp, 1, 10, 2, 1);
    fout.open("test.mp3", ios::out | ios::binary);
}

void KCPClient::start_receive() {
    socket.async_receive_from(
            asio::buffer(receive_buffer), server_endpoint,
            [this](std::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    on_receive(receive_buffer.data(), bytes_recvd);
                }
            }
    );
}

void KCPClient::on_receive(const char *data, size_t length) {
    ikcp_input(kcp, receive_buffer.data(), length);

    int recv_size;
    while((recv_size = ikcp_recv(kcp, file_buffer.data(), file_buffer.size())) > 0) {
        if (file_buffer[length - 1] == 'd' && file_buffer[length - 2] == 'n' && file_buffer[length - 3] == 'e') {
            fout.write(file_buffer.data(), recv_size - 3);
            fout.close();
            return;
        }
        fout.write(file_buffer.data(), recv_size);
    }

    start_receive();
}

int KCPClient::udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
    auto *client = (KCPClient*)user;
    client->socket.async_send_to(asio::buffer(buf, len), client->server_endpoint, [](std::error_code ec, std::size_t bytes_sent) {
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
    bool should_close = data_wrapper->should_close.get_and_reset() != std::nullopt;
    if (should_close) {
        io_context.stop();
        return;
    }
    ikcp_update(kcp, iclock());

    // refresh timer
    timer->expires_after(asio::chrono::milliseconds(10));
    timer->async_wait([this](std::error_code ec) {
        if (!ec) {
            this->update();
        }
    });
}



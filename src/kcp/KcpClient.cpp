//
// Created by 86137 on 2023/12/17.
//

#include "KcpClient.h"

KCPClient::KCPClient(asio::io_context &io_context, const std::string &server_ip, unsigned short server_port)
        : io_context(io_context), socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)),
          server_endpoint(asio::ip::address::from_string(server_ip), server_port) {

    kcp = ikcp_create(0x11223344, (void*)this);
    ikcp_setoutput(kcp, &KCPClient::udp_output);
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
    ikcp_input(kcp, data, length);
    int recv_size = ikcp_recv(kcp, receive_buffer.data(), receive_buffer.size());
    if (recv_size > 0) {
        std::cout << "receive data: " << receive_buffer.data() << std::endl;
        should_exit = true;
    }
    /*this->io_context.stop();*/
}



//
// Created by 86137 on 2023/12/28.
//

#include "api/TcpClient.h"
#include "iostream"
#include "detail/error_handler.h"

TCPClient::TCPClient(const string &server_ip, unsigned short server_port,
                     DataWrapper *data_wrapper, ApplicationProtocolBase* protocol): socket(io_context), data_wrapper
                     (data_wrapper), resolver(io_context) {
    this->application_protocol = protocol;
    auto endpoint = resolver.resolve(server_ip, std::to_string(server_port));
    asio::async_connect(socket, endpoint, [](std::error_code ec, const asio::ip::tcp::endpoint& endpoint) {
        if (!ec) {
            std::cout << "connected to server: " << endpoint << std::endl;
        } else {
            std::cout << "connect error: " << ec.message() << std::endl;
        }
    });
    this->timer = new asio::steady_timer(io_context, asio::chrono::milliseconds(10));
    timer->async_wait([this](std::error_code ec) {
        if (!ec) {
            this->update();
        }
    });
    start_receive();
}

void TCPClient::send(const char* data, size_t length) {
    socket.async_write_some(asio::buffer(data, length), [](std::error_code ec, std::size_t
    bytes_sent) {
        if (ec) {
            std::cout << "send error: " << ec.message() << std::endl;
        }
        if (!ec && bytes_sent > 0) {
            std::cout << "send data: " << bytes_sent << std::endl;
        }
    });
}

void TCPClient::start_receive() {
    receive_buffer.fill(0);
    socket.async_read_some(asio::buffer(receive_buffer), [this](std::error_code ec, std::size_t bytes_recvd) {
        if (!ec && bytes_recvd > 0) {
            if (application_protocol->process_segment(receive_buffer.data(), bytes_recvd)) {
                data_wrapper->recv_queue.push(application_protocol->get_params());
                application_protocol->reset();
            }
        }
    });
    start_receive();
}

void TCPClient::update() {
    should_exit = data_wrapper->should_close.get_and_reset() != std::nullopt;
    if (should_exit) {
        io_context.stop();
        data_wrapper->should_close.set(true); // We do not know either the other client has closed the connection.
        return;
    }

    optional<string> request;
    int segment_size = 1000; // leave some space for tcp header and '\0'
    while ((request = data_wrapper->tcp_send_queue.try_pop()) != std::nullopt) {
        string request_buffer;
        request_buffer.resize(segment_size);
        for (int i = 0; i < request.value().size(); i += segment_size) {
            int size = std::min(segment_size, (int) request.value().size() - i);
            std::copy(request.value().begin() + i, request.value().begin() + i + size, request_buffer.begin());
            send(request_buffer.data(), size);
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

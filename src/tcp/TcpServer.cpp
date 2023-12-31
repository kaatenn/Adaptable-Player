#include "api/TcpServer.h"
#include "detail/error_handler.h"
#include <iostream>

using namespace kaatenn;

TCPServer::TCPServer(unsigned short port, ApplicationProtocolBase* protocol) :socket(io_context),
remote_endpoint(asio::ip::tcp::v4(), port), acceptor(io_context), application_protocol(protocol) {
    check_port(port);
    acceptor.open(remote_endpoint.protocol());
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(remote_endpoint);
    asio::error_code ec;
    acceptor.listen(asio::socket_base::max_listen_connections, ec);
    /*if (ec) {
        std::cerr << "acceptor listen error: " << ec.message() << std::endl;
        exit(1);
    } else {
        std::cout << "acceptor listen on port: " << port << std::endl;
    }*/
    start_receive();
    asio_thread = std::thread([this]() { io_context.run(); });
}

void TCPServer::run_server() {
    asio_thread.join();
}

void TCPServer::start_receive() {
    acceptor.async_accept(socket, [this](const asio::error_code& ec) {
        if (!ec) {
            std::make_shared<TCPConnection>(std::move(socket), this->application_protocol)->start(); // shaking hands
        } else {
            std::cerr << "accept error: " << ec.message() << std::endl;
        }
    });
}


void TCPConnection::start() {
    do_read();
}

void TCPConnection::do_read() {
    auto self(shared_from_this());
    receive_buffer.fill(0);
    socket.async_read_some(asio::buffer(receive_buffer), [this, self](std::error_code ec,
            std::size_t
    byte_recv) {
        if (!ec) {
            if (application_protocol->process_segment(receive_buffer.data(), byte_recv)) {
                string response = application_protocol->get_response();
                send(response.data(), response.size());
                application_protocol->reset();
            }
            start();
        } else {
            std::cerr << "read error: " << ec.message() << std::endl;
            exit(EXIT_FAILURE);
        }
    });
}

void TCPConnection::send(const char *data, size_t length) {
    auto self(shared_from_this());
    socket.async_write_some(asio::buffer(data, length), [this, self](std::error_code ec, std::size_t bytes_sent) {
        if (ec) {
            std::cout << "send error: " << ec.message() << std::endl;
        }
        if (!ec && bytes_sent > 0) {
            /*std::cout << "send data: " << bytes_sent << std::endl;*/
        }
    });
}

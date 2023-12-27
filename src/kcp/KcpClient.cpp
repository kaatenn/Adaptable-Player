//
// Created by 86137 on 2023/12/17.
//

#include "KcpClient.h"
#include "Connection.hpp"

KCPClient::KCPClient(asio::io_context &io_context, const std::string &server_ip, unsigned short server_port,
                     DataWrapper *data_wrapper)
        : io_context(io_context), socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)),
          server_endpoint(asio::ip::address::from_string(server_ip), server_port), data_wrapper(data_wrapper) {

    kcp = ikcp_create(0x11223344, (void *) this);
    socket.connect(server_endpoint);
    ikcp_setoutput(kcp, &KCPClient::udp_output);

    this->timer = new asio::steady_timer(io_context, asio::chrono::milliseconds(10));
    timer->async_wait([this](std::error_code ec) {
        if (!ec) {
            this->update();
        }
    });
    ikcp_nodelay(kcp, 1, 10, 2, 1);
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
    // MANAGE ONLY ONE PACKET !!!

    int recv_size;

    while ((recv_size = ikcp_recv(kcp, file_buffer.data(), file_buffer.size())) > 0) {
        // When we could successfully receive a legal json, we assert the ending of the response.
        ending_asserting_string.append(file_buffer.data(), recv_size);
        if (waiting_file_name.empty()){
            std::fill(file_buffer.begin(), file_buffer.end(), 0);
            Connection response = Connection::from_json(ending_asserting_string);
            check_need_file(response);
            if (response.get_url() != "error" && waiting_file_name.empty()) {
                std::cout << "receive data: " << ending_asserting_string << std::endl;
                data_wrapper->recv_queue.push(response.to_json());
                ending_asserting_string.clear();
            }
        } else {
            // TODO: Repair a bug(will crash after receiving a file)
            waiting_file_size -= recv_size;
            if (!fout.is_open())
                fout.open(waiting_file_name, ios::binary | ios::out);
            fout.write(file_buffer.data(), recv_size);
            std::cout << "receive file: " << recv_size << std::endl;
            std::fill(file_buffer.begin(), file_buffer.end(), 0);
            if (waiting_file_size <= 0) {
                fout.close();
                waiting_file_name.clear();
                ending_asserting_string.clear();
                data_wrapper->recv_queue.push(Connection(waiting_url, {"file received"}).to_json());
                waiting_url.clear();
                std::cout << "file received" << std::endl;
            }
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
    bool should_close = data_wrapper->should_close.get_and_reset() != std::nullopt;
    if (should_close) {
        io_context.stop();
        return;
    }
    ikcp_update(kcp, iclock());

    // read request from send_queue
    optional<string> request;
    int segment_size = 900; // leave some space for kcp header and '\0'
    while ((request = data_wrapper->send_queue.try_pop()) != nullopt) {
        vector<char> request_buffer;
        request_buffer.resize(request.value().size());
        request_buffer.assign(request->begin(), request->end());
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

void KCPClient::send(const string &url, const char *data, size_t length) {
    ikcp_send(kcp, data, length);
}

void KCPClient::check_need_file(const Connection &connection) {
    // Three params required: str("need file"), str(file_name), str(file_size)
    if (connection.get_params().size() != 3)
        return;
    if (connection.get_params()[0] != "file")
        return;
    waiting_file_name = connection.get_params()[1];
    waiting_file_size = std::stoi(connection.get_params()[2]);
    waiting_url = connection.get_url();
}



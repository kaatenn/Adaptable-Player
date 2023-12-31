#include "thread"
#include <string>
#include <iostream>
#include <exception>

#include "api/KcpClient.h"
#include "example/ui.hpp"
#include "Data/DataWrapper.hpp"
#include "api/TcpClient.h"

using std::string;
int main() {
    try {
        // data init
        DataWrapper data_wrapper;

        // ui init
        std::thread ui_thread([&data_wrapper]() {
            render(&data_wrapper);
        });
        // kcp asio init
        asio::io_context kcp_io_context;

        EP ep_kcp;
        unsigned short port = 12345;
        std::string server_ip = "127.0.0.1";
        KCPClient client(server_ip, port, &data_wrapper, 0x11223344, &ep_kcp);
        std::cout << "[kcp init]success" << std::endl;

        // tcp asio init
        EP ep_tcp;
        unsigned short tcp_port = 12346;
        std::string tcp_server_ip = "127.0.0.1";
        asio::io_context tcp_io_context;
        TCPClient tcp_client(tcp_server_ip, tcp_port, &data_wrapper, &ep_tcp);
        std::cout << "[tcp init]success" << std::endl;

        // thread init
        try {
            std::thread kcp_asio_thread([&kcp_io_context]() { kcp_io_context.run(); });
            std::thread tcp_asio_thread([&tcp_io_context]() { tcp_io_context.run(); });
            // thread join
            kcp_asio_thread.join();
            tcp_asio_thread.join();
            ui_thread.join();
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

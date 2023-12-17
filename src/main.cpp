#include "thread"
#include <string>
#include <iostream>
#include <exception>

#include "asio/asio.hpp"

#include "kcp/KcpClient.h"


using std::string;
int main() {
    try {
        asio::io_context io_context;

        unsigned short port = 12345;
        std::string server_ip = "127.0.0.1";
        KCPClient client(io_context, server_ip, port);

        client.send("hello", 5);
        std::thread client_thread([&]() {
            while (true) {
                client.update();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        client_thread.join();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

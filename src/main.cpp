#include "thread"
#include <string>
#include <iostream>
#include <exception>

#include "kcp/KcpClient.h"
#include "ui/ui.hpp"

using std::string;
int main() {
    try {
        // ui init
        std::thread ui_thread([]() {
            render();
        });
        // asio init
        asio::io_context io_context;

        unsigned short port = 12345;
        std::string server_ip = "127.0.0.1";
        KCPClient client(io_context, server_ip, port);


        client.start_receive();
        const char* msg = "hello";
        client.send(msg, strlen(msg) + 1);
        std::thread asio_thread([&io_context]() { io_context.run(); });
        /*io_context.run();*/

        // thread join
        asio_thread.join();
        ui_thread.join();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

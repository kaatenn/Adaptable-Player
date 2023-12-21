#include "thread"
#include <string>
#include <iostream>
#include <exception>

#include "kcp/KcpClient.h"

using std::string;
int main() {
    try {
        asio::io_context io_context;

        unsigned short port = 12345;
        std::string server_ip = "127.0.0.1";
        KCPClient client(io_context, server_ip, port);


        client.start_receive();
        const char* msg = "hello";
        client.send(msg, strlen(msg) + 1);
        io_context.run();

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

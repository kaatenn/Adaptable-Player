#include "thread"
#include <string>
#include <iostream>
#include <exception>

#include "kcp/KcpClient.h"
#include "ui/ui.hpp"
#include "Data/DataWrapper.hpp"
#include "kcp/Connection.hpp"

using std::string;
int main() {
    try {
        // data init
        DataWrapper data_wrapper;

        // ui init
        std::thread ui_thread([&data_wrapper]() {
            render(&data_wrapper);
        });
        // asio init
        asio::io_context io_context;

        unsigned short port = 12345;
        std::string server_ip = "127.0.0.1";
        KCPClient client(io_context, server_ip, port, &data_wrapper);


        client.start_receive();
        // TODO: Refactor this part to ui.hpp by sending to send_queue in data_wrapper
        string url = "music_list";
        Connection request(url);
        client.send(request.get_url(), request.to_json().c_str(), request.get_sending_length());
        std::thread asio_thread([&io_context]() { io_context.run(); });

        // thread join
        asio_thread.join();
        ui_thread.join();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

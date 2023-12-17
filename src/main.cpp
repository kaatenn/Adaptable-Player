#include <asio/asio.hpp>
#include "ikcp.h"
#include "ui/utils/GuiLoader.hpp"
#include <iostream>
#include <cstring>

using asio::ip::udp;

// KCP的输出回调函数，用于发送UDP数据
int udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
    std::cout << "[stepin]success" << std::endl;
    udp::socket* socket = reinterpret_cast<udp::socket*>(user);
    std::cout << "[socket]success" << std::endl;
    udp::endpoint receiver_endpoint(asio::ip::address::from_string("127.0.0.1"), 12345);
    std::cout << "[endpoint]success" << std::endl;
    socket->send_to(asio::buffer(buf, len), receiver_endpoint);
    std::cout << "[send]success" << std::endl;
    return 0;
}

void client_demo(asio::io_context& io_context) {
    udp::socket socket(io_context, udp::endpoint(udp::v4(), 0));
    ikcpcb *kcp = ikcp_create(0x11223344, (void*)&socket);
    kcp->output = udp_output;

    const char *message = "Hello from Client!";
    ikcp_send(kcp, message, strlen(message));

    ikcp_update(kcp, static_cast<IUINT32>(0.1));
    ikcp_release(kcp);
}

int main() {
    asio::io_context io_context;
    client_demo(io_context);

    Gui_Loader gui_loader;
    gui_loader.init();
    gui_loader.run();
    gui_loader.close();

    return 0;
}

//
// Created by 86137 on 2023/12/30.
//
#include <iostream>
#include "detail/error_handler.h"


/**
 * Checks if the specified port is already in use.
 *
 * This function iterates through the list of currently used ports and
 * compares each with the specified port. If a match is found, it indicates
 * that the port is already in use, prints an error message, and terminates
 * the program. Otherwise, it adds the port to the list of used ports.
 *
 * @param port The port number to check.
 */

void check_port(unsigned int port) {
    for (auto &using_port: using_ports) {
        if (using_port == port) {
            std::cerr << "Port " << port << " is already in use." << std::endl;
            abort();
        }
    }
    using_ports.push_back(port);
}
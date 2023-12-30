#ifndef ADAPTABLE_SERVER_ERROR_HANDLER_H
#define ADAPTABLE_SERVER_ERROR_HANDLER_H

#include <stdexcept>
#include <string>
#include "vector"

using std::vector;

static vector<unsigned int> using_ports;

void check_port(unsigned int port);

#endif //ADAPTABLE_SERVER_ERROR_HANDLER_H

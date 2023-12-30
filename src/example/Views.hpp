//
// Created by 86137 on 2023/12/24.
//

#ifndef ADAPTABLE_SERVER_VIEWS_HPP
#define ADAPTABLE_SERVER_VIEWS_HPP

#include "string"
#include "vector"
#include "sstream"
#include "fstream"
#include "detail/type_definer.hpp"
#include "EP.h"

std::string get_music_list([[maybe_unused]]EP& Ep);
std::string send_file(EP& Ep);

#endif //ADAPTABLE_SERVER_VIEWS_HPP

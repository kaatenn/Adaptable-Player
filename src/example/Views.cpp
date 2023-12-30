//
// Created by 86137 on 2023/12/29.
//

#include "Views.hpp"
#include <filesystem>
#include "json.hpp"

std::string get_music_list([[maybe_unused]]EP& ep) {
    vector<std::string> music_list;

    for (const auto& entry : std::filesystem::directory_iterator("music")) {
        if (entry.is_regular_file() && entry.path().extension() == ".mp3") {
            music_list.push_back(entry.path().filename().string());
        }
    }

    nlohmann::json json;
    json["music_list"] = music_list;
    std::string res_params = json.dump();
    return res_params;
}


std::string send_file(EP& ep) {
    nlohmann::json json;
    std::string params = ep.get_params();
    json = nlohmann::json::parse(params);
    std::string file_name = json["file_name"];
    std::ifstream file("music/" + file_name, std::ios::binary);
    std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    nlohmann::json res_json;
    res_json["file_name"] = file_name;
    std::string res_params = res_json.dump();
    ep.set_res_data_length(res_params.size());
    return res_params + file_content;
}
//
// Created by 86137 on 2023/12/24.
//

#ifndef ADAPTABLE_UPLOADER_CONNECTION_HPP
#define ADAPTABLE_UPLOADER_CONNECTION_HPP

#include <utility>

#include "json.hpp"
#include "string"
#include "vector"

using std::string, std::vector;

class Connection {
public:
    explicit Connection(string in_url, const vector<string> &in_params = vector<string>()) : url(std::move(in_url)),
                                                                                             params(in_params) {}

    static Connection from_json(const string& json_str) {
        try {
            auto json = nlohmann::json::parse(json_str);
            return Connection(json["url"], json["params"]);
        } catch (nlohmann::json::exception& e) {
            return Connection("error", {e.what()});
        }
    }

    [[nodiscard]] string get_url() const {
        return this->url;
    }

    void set_url(string in_url) {
        this->url = std::move(in_url);
    }

    int get_sending_length() {
        return static_cast<int>(this->to_json().length()) + 1;
    }

    void set_params(vector<string> in_params) {
        this->params = std::move(in_params);
    }

    [[nodiscard]] vector<string> get_params() const {
        return this->params;
    }

    [[nodiscard]] string to_json() const {
        nlohmann::json json;
        json["url"] = this->url;
        json["params"] = this->params;
        return json.dump();
    }

    vector<char> to_buffer() {
        string json = this->to_json();
        vector<char> buffer(json.begin(), json.end());
        return buffer;
    }

private:
    string url;
    vector<string> params;
};

#endif //ADAPTABLE_UPLOADER_CONNECTION_HPP

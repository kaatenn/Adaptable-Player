//
// Created by 86137 on 2023/12/30.
//

#include <iostream>
#include "EP.h"
#include "UrlMap.hpp"
#include "json.hpp"

bool EP::process_segment(char *segment, int recv_size) {
    receive_buffer.append(segment, recv_size);
    std::cout << "recvd: " << recv_size << std::endl;
    if (length == 0 || length == 6) {
        // length == 6 means that the init of the instance with length 0 + 0 + 0 + 6
        if (receive_buffer.size() >= 4) {
            length = *(int *) receive_buffer.data();
            receive_buffer.erase(0, 4);
        } else {
            return false;
        }
    }
    if (url_length == 0) {
        if (receive_buffer.size() >= 1) {
            url_length = *(char *) receive_buffer.data();
            receive_buffer.erase(0, 1);
        } else {
            return false;
        }
    }
    if (param_length == 0) {
        if (receive_buffer.size() >= 1) {
            param_length = *(char *) receive_buffer.data();
            receive_buffer.erase(0, 1);
        } else {
            return false;
        }
    }
    if (url.empty()) {
        if (receive_buffer.size() >= url_length) {
            url = receive_buffer.substr(0, url_length);
            receive_buffer.erase(0, url_length);
        } else {
            return false;
        }
    }
    if (params.empty()) {
        if (receive_buffer.size() >= param_length) {
            params = receive_buffer.substr(0, param_length);
            receive_buffer.erase(0, param_length);
        } else {
            return false;
        }
    }
    file_length = length - url_length - param_length - 6;
    if (file_stream.empty()) {
        if (receive_buffer.size() >= file_length) {
            file_stream = receive_buffer.substr(0, file_length);
            receive_buffer.erase(0, file_length);
            if (!params.empty()) {
                nlohmann::json json;
                json = nlohmann::json::parse(params);
                if (json.contains("file_name")){
                    std::string file_name = json["file_name"];
                    std::ofstream fout("music/" + file_name, std::ios::binary);
                    fout.write(file_stream.data(), file_stream.size());
                    fout.close();
                }
            }
        } else {
            return false;
        }
    }
    return true;
}

void EP::set_res_data_length(int length) {
    this->res_param_length = length;
}

std::string EP::get_params() const {
    return params;
}

void EP::generate_response() {
    std::string res_data = url_map[url](*this);
    int res_file_len = res_data.size() - res_param_length;
    int res_len = 6 + url_length + res_param_length + res_file_len;
    std::string res;
    res.resize(0);
    res.append((char *) &res_len, 4);
    res.append((char *) &url_length, 1);
    res.append((char *) &res_param_length, 1);
    res.append(url);
    res.append(res_data); // res_data = res_params + res_file_stream
    this->response = res;
}

std::string EP::get_response() {
    generate_response();
    return response;
}

std::string EP::serialize() const {
    std::string result;
    result.append((char *) &length, 4);
    result.append((char *) &url_length, 1);
    result.append((char *) &param_length, 1);
    result.append(url);
    result.append(params);
    result.append(file_stream);
    return result;
}

void EP::error_handler() {
    // This is just a demo, so we do not need to handle errors.
}

void EP::reset() {
    length = 0;
    url_length = 0;
    param_length = 0;
    file_length = 0;
    res_param_length = 0;
    receive_buffer.clear();
    url.clear();
    params.clear();
    file_stream.clear();
    response.clear();
    if (fin.is_open())
        fin.close();
}

string EP::get_url() const {
    return url;
}

EP::EP(std::string url, std::string params, std::string file_stream) : url(std::move(url)), params(std::move(params)),
                                                                       file_stream(std::move(file_stream)) {
    url_length = this->url.size();
    param_length = this->params.size();
    file_length = this->file_stream.size();
    length = 6 + url_length + param_length + file_length;
}

EP EP::deserialize(const string &data) {
    EP ep;
    ep.length = *(int *) data.data();
    ep.url_length = *(char *) (data.data() + 4);
    ep.param_length = *(char *) (data.data() + 5);
    ep.url = data.substr(6, ep.url_length);
    ep.params = data.substr(6 + ep.url_length, ep.param_length);
    ep.file_stream = data.substr(6 + ep.url_length + ep.param_length);
    return ep;
}

std::string EP::get_file_stream() const {
    return file_stream;
}

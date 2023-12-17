//
// Created by 86137 on 2023/12/17.
//

#ifndef ADAPTABLE_UPLOADER_PROTOCOLSELECTOR_H
#define ADAPTABLE_UPLOADER_PROTOCOLSELECTOR_H

#include <vector>
#include <string>

#include "imgui.h"

using std::vector;
using std::string;

class Protocol_Selector {
public:
    void render();

    /**
     * @brief Add a protocol to the protocol list.
     * @param protocol
     * @author kaatenn
     * @date 2023/12/17
     */
    [[maybe_unused]] void add_protocol(const string& protocol) {
        this->protocols.push_back(protocol);
    }

private:
    vector<string> protocols = { "KCP", "TCP", "RTP" };
    int current_protocol_index = 0;
    bool auto_select = false;

};


#endif //ADAPTABLE_UPLOADER_PROTOCOLSELECTOR_H

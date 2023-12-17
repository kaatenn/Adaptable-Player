//
// Created by 86137 on 2023/12/17.
//

#include "ProtocolSelector.h"

void Protocol_Selector::render() {
    ImGui::Begin("Protocol Selector");

    if (ImGui::Checkbox("Auto Select", &auto_select)) {
        // TODO: Auto select protocol
    }

    if (!auto_select) {
        if (ImGui::BeginCombo("Protocol", protocols[current_protocol_index].c_str())) {
            for (int i = 0; i < protocols.size(); i++) {
                bool is_selected = (current_protocol_index == i);
                if (ImGui::Selectable(protocols[i].c_str(), is_selected))
                    current_protocol_index = i;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
}

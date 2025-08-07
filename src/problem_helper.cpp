

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#include "json.hpp"
#include "problem.h"

ProblemHelper::ProblemHelper(const json& data) {
    std::vector<std::string> shift_types = data["shift_types"];
    for (int i = 0; i < shift_types.size(); i++) {
        shift_type_to_index[shift_types[i]] = i;
    }

    std::vector<std::string> age_groups = data["age_groups"];
    for (int i = 0; i < age_groups.size(); i++) {
        age_group_to_index[age_groups[i]] = i;
    }
    
    gender_to_index = {{"A", 0}, {"B", 1}};

    std::vector<json> rooms = data["rooms"];
    for (int i = 0; i < rooms.size(); i++) {
        room_id_to_index[rooms[i]["id"]] = i;
    }

    std::vector<json> surgeons = data["surgeons"];
    for (int i = 0; i < surgeons.size(); i++) {
        surgeon_id_to_index[surgeons[i]["id"]] = i;
    }
}
#ifndef PROBLEM_HELPER_H
#define PROBLEM_HELPER_H

#include <map>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

class ProblemHelper {
public:
    std::map<std::string, int> shift_type_to_index;
    std::map<std::string, int> age_group_to_index;
    std::map<std::string, int> gender_to_index;

    std::map<std::string, int> room_id_to_index;
    std::map<std::string, int> surgeon_id_to_index;

    ProblemHelper(const ProblemHelper&) = delete;
    ProblemHelper(const json& data);
};

#endif
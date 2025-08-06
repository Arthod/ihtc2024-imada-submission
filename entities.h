#ifndef HEALTHCARE_ENTITIES_H
#define HEALTHCARE_ENTITIES_H

#include <string>
#include <vector>
#include <unordered_map>
#include "json.hpp"

#include "problem_helper.h"

using json = nlohmann::json;

class Occupant {
public:
    std::string id;
    int gender;
    int age_group;
    int length_of_stay;
    std::vector<int> workload_produced;
    std::vector<int> skill_level_required;
    int room_index;

    static Occupant from_dict(const json& data, const ProblemHelper& helper);
};

class Patient {
public:
    std::string id;
    int gender;
    int age_group;
    int length_of_stay;
    std::vector<int> workload_produced;
    std::vector<int> skill_level_required;

    bool mandatory;
    int surgery_release_day;
    int surgery_due_day;
    int surgery_duration;
    int surgeon_index;
    std::vector<int> incompatible_room_indices;

    static Patient from_dict(const json& data, const ProblemHelper& helper, int days);
};

class Surgeon {
public: 
    std::string id;
    std::vector<int> max_surgery_time;

    static Surgeon from_dict(const json& data, const ProblemHelper& helper);
};

class OperatingTheater {
public:
    std::string id;
    std::vector<int> availability;

    static OperatingTheater from_dict(const json& data, const ProblemHelper& helper);
};

class Room {
public:
    std::string id;
    int capacity;

    static Room from_dict(const json& data, const ProblemHelper& helper);
};

class WorkingShift {
public:
    int day;
    int shift_index;
    int max_load;

    static WorkingShift from_dict(const json& data, const ProblemHelper& helper);
};

class Nurse {
public:
    std::string id;
    int skill_level;
    std::vector<WorkingShift> working_shifts;

    static Nurse from_dict(const json& data, const ProblemHelper& helper);
};

class PatientValue {
public:
    int idx;
    int admission_day;
    int room_id;
    int operating_theater_id;

    PatientValue(int idx, int admission_day, int room_id, int operating_theater_id) : admission_day(admission_day), room_id(room_id), operating_theater_id(operating_theater_id) {}
};

#endif
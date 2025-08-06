
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "json.hpp"

#include "problem_helper.h"
#include "entities.h"


Occupant Occupant::from_dict(const json& data, const ProblemHelper& helper) {
    Occupant occupant;
    occupant.id = data["id"];
    occupant.gender = helper.gender_to_index.at(data["gender"]);
    occupant.age_group = helper.age_group_to_index.at(data["age_group"]);
    occupant.length_of_stay = data["length_of_stay"];
    occupant.workload_produced = data["workload_produced"].get<std::vector<int>>();
    occupant.skill_level_required = data["skill_level_required"].get<std::vector<int>>();
    occupant.room_index = helper.room_id_to_index.at(data["room_id"]);
    return occupant;
}

Patient Patient::from_dict(const json& data, const ProblemHelper& helper, int days) {
    Patient patient;
    patient.id = data["id"];
    patient.gender = helper.gender_to_index.at(data["gender"]);
    patient.age_group = helper.age_group_to_index.at(data["age_group"]);
    patient.length_of_stay = data["length_of_stay"];
    patient.workload_produced = data["workload_produced"].get<std::vector<int>>();
    patient.skill_level_required = data["skill_level_required"].get<std::vector<int>>();

    patient.mandatory = data["mandatory"];
    patient.surgery_release_day = data["surgery_release_day"];
    if (patient.mandatory) {
        patient.surgery_due_day = data["surgery_due_day"];
    } else {
        patient.surgery_due_day = days - 1;
    }
    patient.surgery_duration = data["surgery_duration"];
    patient.surgeon_index = helper.surgeon_id_to_index.at(data["surgeon_id"]);
    std::vector<std::string> incompatible_room_ids = data["incompatible_room_ids"];
    for (const auto& room_id : incompatible_room_ids) {
        patient.incompatible_room_indices.push_back(helper.room_id_to_index.at(room_id));
    }
    return patient;
}

Surgeon Surgeon::from_dict(const json& data, const ProblemHelper& helper) {
    Surgeon surgeon;
    surgeon.id = data["id"];
    surgeon.max_surgery_time = data["max_surgery_time"].get<std::vector<int>>();
    return surgeon;
}

OperatingTheater OperatingTheater::from_dict(const json& data, const ProblemHelper& helper) {
    OperatingTheater operating_theater;
    operating_theater.id = data["id"];
    operating_theater.availability = data["availability"].get<std::vector<int>>();
    return operating_theater;
}

Room Room::from_dict(const json& data, const ProblemHelper& helper) {
    Room room;
    room.id = data["id"];
    room.capacity = data["capacity"];
    return room;
}

WorkingShift WorkingShift::from_dict(const json& data, const ProblemHelper& helper) {
    WorkingShift shift;
    shift.day = data["day"];
    shift.shift_index = helper.shift_type_to_index.at(data["shift"]);
    shift.max_load = data["max_load"];
    return shift;
}

Nurse Nurse::from_dict(const json& data, const ProblemHelper& helper) {
    Nurse nurse;
    nurse.id = data["id"];
    nurse.skill_level = data["skill_level"];
    for (const json& shift_data : data["working_shifts"]) {
        nurse.working_shifts.push_back(WorkingShift::from_dict(shift_data, helper));
    }
    return nurse;
}
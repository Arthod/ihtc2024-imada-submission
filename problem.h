#ifndef PROBLEM_H
#define PROBLEM_H

#include <map>
#include <string>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include "json.hpp"
#include <random>

#include "entities.h"

using json = nlohmann::json;


class Problem {
public:
    std::string filename;
    int days;
    int num_skill_levels;
    int num_shift_types;
    std::vector<std::string> shift_types;
    int num_age_groups;
    std::vector<Occupant> occupants;
    std::vector<Patient> patients;
    std::vector<Surgeon> surgeons;
    std::vector<OperatingTheater> operating_theaters;
    std::vector<Room> rooms;
    std::vector<Nurse> nurses;

    std::map<std::string, int> weights;
    std::vector<int> optional_patient_ids;

    std::vector<PatientValue> patient_values;
    std::vector<std::vector<int>> patient_allowed_value_idxs_list;
    std::vector<std::unordered_set<int>> patient_allowed_value_idxs_set;

    std::vector<std::vector<int>> patient_kickable_ids; // patient -> [patient1, ...] patient set to patient1 and patient1 is randomized
    std::vector<std::vector<int>> patient_kickableout_ids; // patient -> [patient1, ...] patient set to patient1 and patient1 is removed
    std::vector<std::vector<int>> patient_swapable_ids; // patient -> [patient1, ...] patient and patient1 can swap

    std::vector<std::vector<std::vector<int>>> day_shift_nurse_ids;

    Problem(const Problem&) = delete;
    Problem(Problem&& other) noexcept;
    Problem();

    static Problem from_textio(const std::string& filename, std::mt19937&);
    void print_short_info() const;
};

#endif
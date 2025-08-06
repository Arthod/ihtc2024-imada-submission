#ifndef SOLUTION_DATA_H
#define SOLUTION_DATA_H

#include <vector>
#include <array>
#include <unordered_set>
#include <string>
#include <fstream>
#include "json.hpp"

#include <random>
#include "problem.h"

using json = nlohmann::json;

class SolutionData {
public:
    const Problem& problem;
    bool is_lazy_copy = true;

    std::vector<std::array<int, 2>> room_day_genders; // EvalElemRoomGenderMix
    std::vector<std::vector<int>> surgeon_day_loads; // EvalElemSurgeonOvertime
    std::vector<std::vector<int>> operating_theater_day_loads; // EvalElemOperatingTheaterOvertime
    std::vector<int> room_day_loads; // EvalElemRoomCapacity
    
    std::vector<std::vector<int>> operating_theater_day_loads2; // EvalElemOpenOperatingTheater
    std::vector<std::vector<std::vector<int>>> room_day_ages; // EvalElemRoomAgeMix
    std::vector<std::vector<int>> room_day_agegroup_max; // EvalElemRoomAgeMix
    std::vector<std::vector<int>> room_day_agegroup_min; // EvalElemRoomAgeMix
    std::vector<std::vector<std::vector<int>>> day_surgeon_operating_theater_loads; // EvalElemSurgeonTransfer
    std::vector<std::vector<std::vector<std::vector<int>>>> room_day_shift_skill_level_patients; // EvalElemRoomSkillLevel
    std::vector<int> patient_nurse_assignments; // EvalElemContinuityOfCare
    std::vector<std::vector<int>> room_day_patients; // EvalElemContinuityOfCare
    std::vector<int> nurse_day_shift_workloads; // EvalElemExcessiveNurseWorkload
    std::vector<int> room_day_shift_patients_workload; // EvalElemExcessiveNurseWorkload

    std::vector<std::vector<int>> operating_theater_day_occupancy_space; // EvalElemOpenOperatingTheaterSpace
    std::vector<std::vector<int>> patient_nurse_assignments2; // EvalElemContinuityOfCareMandatoryPatients
    std::vector<std::vector<std::vector<int>>> room_day_patients2; // EvalElemContinuityOfCareMandatoryPatients


    std::unordered_set<int> patients_scheduled;
    std::vector<int> patient_admission_days;
    std::vector<int> patient_room_ids;
    std::vector<int> patient_operating_theaters;
    std::vector<int> room_day_shift_nurse;
    std::vector<std::vector<std::vector<std::vector<int>>>> nurse_day_shift_rooms;

    SolutionData(const SolutionData& other) = default;


    SolutionData(const Problem& problem, std::unordered_set<int>& patients_scheduled, std::vector<int>& patient_admission_days, std::vector<int>& patient_room_ids, std::vector<int>& patient_operating_theaters, std::vector<int>& room_day_shift_nurse);
    
    SolutionData(const Problem& problem, std::unordered_set<int>& patients_scheduled, std::vector<int>& patient_admission_days, std::vector<int>& patient_room_ids, std::vector<int>& patient_operating_theaters, std::vector<int>& room_day_shift_nurse, std::vector<std::vector<std::vector<std::vector<int>>>>& nurse_day_shift_rooms);

    void write_to_file(const std::string& filename);
    
    SolutionData& operator=(const SolutionData& other);
    
    static SolutionData random_initial_solution(const Problem& problem, std::mt19937& gen);

private:
    std::vector<std::vector<std::vector<std::vector<int>>>> generate_nurse_day_shift_rooms(const Problem& problem);
};

#endif

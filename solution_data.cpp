
#include "solution_data.h"
#include "json.hpp"
#include <fstream>
#include "utils.h"

#include "config.h"

SolutionData::SolutionData(const Problem& problem, std::unordered_set<int>& patients_scheduled, std::vector<int>& patient_admission_days, std::vector<int>& patient_room_ids, std::vector<int>& patient_operating_theaters, std::vector<int>& room_day_shift_nurse) 
    : problem(problem), patients_scheduled(patients_scheduled), patient_admission_days(patient_admission_days), patient_room_ids(patient_room_ids), patient_operating_theaters(patient_operating_theaters), room_day_shift_nurse(room_day_shift_nurse) {
    nurse_day_shift_rooms = generate_nurse_day_shift_rooms(problem);
}

SolutionData::SolutionData(const Problem& problem, std::unordered_set<int>& patients_scheduled, std::vector<int>& patient_admission_days, std::vector<int>& patient_room_ids, std::vector<int>& patient_operating_theaters, std::vector<int>& room_day_shift_nurse, std::vector<std::vector<std::vector<std::vector<int>>>>& nurse_day_shift_rooms) 
    : problem(problem), patients_scheduled(patients_scheduled), patient_admission_days(patient_admission_days), patient_room_ids(patient_room_ids), patient_operating_theaters(patient_operating_theaters), room_day_shift_nurse(room_day_shift_nurse), nurse_day_shift_rooms(nurse_day_shift_rooms) {}

void SolutionData::write_to_file(const std::string& filename) {
    json data;

    // Write patients
    data["patients"] = json::array();
    for (int i = 0; i < problem.patients.size(); i++) {
        const Patient& patient = problem.patients[i];

        json patient_data;
        patient_data["id"] = problem.patients[i].id;

        if (patients_scheduled.find(i) != patients_scheduled.end()) {
            patient_data["admission_day"] = patient_admission_days[i];
            patient_data["room"] = problem.rooms[patient_room_ids[i]].id;
            patient_data["operating_theater"] = problem.operating_theaters[patient_operating_theaters[i]].id;
        } else {
            patient_data["admission_day"] = "none";
        }
        data["patients"].push_back(patient_data);
    }

    // Write nurses
    data["nurses"] = json::array();
    for (int i = 0; i < problem.nurses.size(); i++) {
        Nurse nurse = problem.nurses[i];

        json nurse_data;
        nurse_data["id"] = nurse.id;
        nurse_data["assignments"] = json::array();
        for (int j = 0; j < nurse.working_shifts.size(); j++) {
            WorkingShift shift = nurse.working_shifts[j];
            json shift_data;
            shift_data["day"] = shift.day;
            shift_data["shift"] = problem.shift_types[shift.shift_index];
            shift_data["rooms"] = json::array();
            for (int room : nurse_day_shift_rooms[i][shift.day][shift.shift_index]) {
                shift_data["rooms"].push_back(problem.rooms[room].id);
            }

            nurse_data["assignments"].push_back(shift_data);
        }
        data["nurses"].push_back(nurse_data);
    }

    std::ofstream file(filename);
    file << data.dump(4);
}

SolutionData& SolutionData::operator=(const SolutionData& other) {
    if (this == &other) {
        return *this;
    }
    patients_scheduled = other.patients_scheduled;
    patient_admission_days = other.patient_admission_days;
    patient_room_ids = other.patient_room_ids;
    patient_operating_theaters = other.patient_operating_theaters;
    room_day_shift_nurse = other.room_day_shift_nurse;
    nurse_day_shift_rooms = other.nurse_day_shift_rooms;

    is_lazy_copy = true;

    return *this;
}

SolutionData SolutionData::random_initial_solution(const Problem& problem, std::mt19937& gen) {
    std::unordered_set<int> patients_scheduled;
    std::vector<int> patient_admission_days(problem.patients.size(), -1);
    std::vector<int> patient_room_ids(problem.patients.size(), -1);
    std::vector<int> patient_operating_theaters(problem.patients.size(), -1);

    for (int i = 0; i < problem.patients.size(); i++) {
        int value_idx = problem.patient_allowed_value_idxs_list[i][random_int(gen, 0, problem.patient_allowed_value_idxs_list[i].size() - 1)];
        const auto& patient_value = problem.patient_values[value_idx];

        patient_admission_days[i] = patient_value.admission_day;
        patient_room_ids[i] = patient_value.room_id;
        patient_operating_theaters[i] = patient_value.operating_theater_id;

        patients_scheduled.insert(i);
    }

    
    std::vector<int> room_day_shift_nurse(problem.rooms.size() * problem.days * problem.num_shift_types, 0);
    for (int day = 0; day < problem.days; day++) {
        for (int shift = 0; shift < problem.num_shift_types; shift++) {
            const auto& nurses = problem.day_shift_nurse_ids[day][shift];
            
            for (int room = 0; room < problem.rooms.size(); room++) {
                int idx = room * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                room_day_shift_nurse[idx] = nurses[random_int(gen, 0, nurses.size() - 1)];
            }
        }
    }

    return SolutionData(problem, patients_scheduled, patient_admission_days, patient_room_ids, patient_operating_theaters, room_day_shift_nurse);
}

std::vector<std::vector<std::vector<std::vector<int>>>> SolutionData::generate_nurse_day_shift_rooms(const Problem& problem) {
    auto nurse_day_shift_rooms = std::vector<std::vector<std::vector<std::vector<int>>>>(problem.nurses.size(), std::vector<std::vector<std::vector<int>>>(problem.days, std::vector<std::vector<int>>(problem.num_shift_types)));
    
    for (int room = 0; room < problem.rooms.size(); room++) {
        for (int day = 0; day < problem.days; day++) {
            for (int shift = 0; shift < problem.num_shift_types; shift++) {

                int idx = room * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                int nurse_id = room_day_shift_nurse[idx];
                
                int shifts_found = 0;
                for (int i = 0; i < problem.nurses[nurse_id].working_shifts.size(); i++) {
                    int w_day = problem.nurses[nurse_id].working_shifts[i].day;
                    int w_shift = problem.nurses[nurse_id].working_shifts[i].shift_index;

                    if (day == w_day && shift == w_shift) {
                        nurse_day_shift_rooms[nurse_id][day][shift].push_back(room);
                        shifts_found++;
                    }
                }
#if DEBUG >= 2
                assert(shifts_found <= 1);
#endif
            }
        }
    }

    return nurse_day_shift_rooms;
}
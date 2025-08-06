
#include "problem.h"
#include "json.hpp"
#include "entities.h"
#include <random>
#include "config.h"

Problem::Problem() {}

Problem Problem::from_textio(const std::string& filename, std::mt19937& gen) {
    std::cout << "Reading file: " << filename << std::endl;
    std::ifstream file(filename);
    json data;
    file >> data;

    Problem p;
    p.filename = filename;
    p.days = data["days"];
    p.num_skill_levels = data["skill_levels"];

    ProblemHelper helper = ProblemHelper(data);
    
    p.num_shift_types = data["shift_types"].size();
    p.shift_types = data["shift_types"].get<std::vector<std::string>>();
    p.num_age_groups = data["age_groups"].size();

    for (const json& occupant_data : data["occupants"]) {
        p.occupants.push_back(Occupant::from_dict(occupant_data, helper));
    }

    for (const json& patient_data : data["patients"]) {
        p.patients.push_back(Patient::from_dict(patient_data, helper, p.days));
    }

    for (const json& surgeon_data : data["surgeons"]) {
        p.surgeons.push_back(Surgeon::from_dict(surgeon_data, helper));
    }

    for (const json& operating_theater_data : data["operating_theaters"]) {
        p.operating_theaters.push_back(OperatingTheater::from_dict(operating_theater_data, helper));
    }

    for (const json& room_data : data["rooms"]) {
        p.rooms.push_back(Room::from_dict(room_data, helper));
    }

    for (const json& nurse_data : data["nurses"]) {
        p.nurses.push_back(Nurse::from_dict(nurse_data, helper));
    }

    p.weights = data["weights"];

    // Patient values
    for (int room = 0; room < p.rooms.size(); room++) {
        for (int day = 0; day < p.days; day++) {
            for (int t = 0; t < p.operating_theaters.size(); t++) {
                int idx = room * p.days * p.operating_theaters.size() + day * p.operating_theaters.size() + t;
                p.patient_values.push_back(PatientValue(idx, day, room, t));
            }
        }
    }

    std::vector<std::vector<int>> room_day_occupant_capacity(p.rooms.size(), std::vector<int>(p.days, 0));
    for (int i = 0; i < p.occupants.size(); i++) {
        const Occupant& occupant = p.occupants[i];
        for (int day = 0; day < occupant.length_of_stay; day++) {
            room_day_occupant_capacity[occupant.room_index][day]++;
        }
    }
    
    std::vector<std::vector<std::array<int, 2>>> room_day_occupant_gender_count(p.rooms.size(), std::vector<std::array<int, 2>>(p.days, {0, 0}));
    for (int i = 0; i < p.occupants.size(); i++) {
        const Occupant& occupant = p.occupants[i];
        for (int day = 0; day < occupant.length_of_stay; day++) {
            room_day_occupant_gender_count[occupant.room_index][day][occupant.gender]++;
        }
    }

    // Helpers
    int total_allowed_values = 0;
    for (int i = 0; i < p.patients.size(); i++) {
        const Patient& patient = p.patients[i];
        p.patient_allowed_value_idxs_list.push_back(std::vector<int>());
        p.patient_allowed_value_idxs_set.push_back(std::unordered_set<int>());

        for (int room = 0; room < p.rooms.size(); room++) {
            if (std::find(patient.incompatible_room_indices.begin(), patient.incompatible_room_indices.end(), room) != patient.incompatible_room_indices.end()) {
                continue;
            }

            int start = patient.surgery_release_day;
            int end = patient.surgery_due_day;

            for (int day = start; day < end + 1; day++) {
                const Surgeon& surgeon = p.surgeons[patient.surgeon_index];
                if (surgeon.max_surgery_time[day] < patient.surgery_duration) {
                    continue;
                }

                if (room_day_occupant_capacity[room][day] >= p.rooms[room].capacity) {
                    continue;
                }

                if (room_day_occupant_gender_count[room][day][1 - patient.gender] > 0) {
                    continue;
                }

                for (int t = 0; t < p.operating_theaters.size(); t++) {
                    int patient_value_idx = room * p.days * p.operating_theaters.size() + day * p.operating_theaters.size() + t;
                    const OperatingTheater& ot = p.operating_theaters[t];
                    if (ot.availability[day] < patient.surgery_duration) {
                        continue;
                    }

                    total_allowed_values++;
                    p.patient_allowed_value_idxs_list[i].push_back(patient_value_idx);
                    p.patient_allowed_value_idxs_set[i].insert(patient_value_idx);
                }
            }
        }

        // Optional patients
        if (!patient.mandatory) {
            p.optional_patient_ids.push_back(i);
        }
    }

#if DEBUG >= 1
    int total_values = 0;
    for (int i = 0; i < p.patients.size(); i++) {
        const Patient& patient = p.patients[i];

        for (int room = 0; room < p.rooms.size(); room++) {
            if (std::find(patient.incompatible_room_indices.begin(), patient.incompatible_room_indices.end(), room) != patient.incompatible_room_indices.end()) {
                continue;
            }

            int start = patient.surgery_release_day;
            int end = patient.surgery_due_day;

            for (int day = start; day < end + 1; day++) {
                for (int t = 0; t < p.operating_theaters.size(); t++) {
                    total_values++;
                }
            }
        }
    }
    std::cout << "Total values: " << total_values << ", Total allowed values: " << total_allowed_values << std::endl;

    // Check for empty patient value lists
    for (int i = 0; i < p.patients.size(); i++) {
        if (p.patient_allowed_value_idxs_list[i].size() == 0) {
            if (p.patients[i].mandatory) {
                std::cout << "Mandatory patient " << i << " has no allowed values" << std::endl;
                exit(1);
            } else {
                std::cout << "Optional patient " << i << " has no allowed values" << std::endl;
                exit(1);
            }
        } else
        if (p.patient_allowed_value_idxs_list[i].size() == 1) {
            if (p.patients[i].mandatory) {
                std::cout << "Mandatory patient " << i << " has only one allowed value" << std::endl;
                exit(1);
            } else {
                std::cout << "Optional patient " << i << " has only one allowed value" << std::endl;
                exit(1);
            }
        }
    }
#endif

    // day_shift_nurse_ids
    p.day_shift_nurse_ids = std::vector<std::vector<std::vector<int>>>(p.days, std::vector<std::vector<int>>(p.num_shift_types, std::vector<int>()));
    for (int i = 0; i < p.nurses.size(); i++) {
        const Nurse& nurse = p.nurses[i];

        for (int j = 0; j < nurse.working_shifts.size(); j++) {
            const WorkingShift& shift = nurse.working_shifts[j];

            p.day_shift_nurse_ids[shift.day][shift.shift_index].push_back(i);
        }
    }

    // patient_kickable_ids
    p.patient_kickable_ids = std::vector<std::vector<int>>(p.patients.size());
    for (int i = 0; i < p.patients.size(); i++) {
        for (int j = 0; j < p.patients.size(); j++) {
            if (i == j) {
                continue;
            }

            bool overlap = false;
            for (int value_idx : p.patient_allowed_value_idxs_list[i]) {
                if (p.patient_allowed_value_idxs_set[j].find(value_idx) != p.patient_allowed_value_idxs_set[j].end()) {
                    overlap = true;
                    break;
                }
            }
            if (!overlap) {
                continue;
            }

            p.patient_kickable_ids[i].push_back(j);
        }
    }

    // patient_kickableout_ids
    p.patient_kickableout_ids = std::vector<std::vector<int>>(p.patients.size());
    for (int i = 0; i < p.patients.size(); i++) {
        for (int j : p.patient_kickable_ids[i]) {
            if (p.patients[j].mandatory) {
                continue;
            }

            p.patient_kickableout_ids[i].push_back(j);
        }
    }

    // patient_swapable_ids
    p.patient_swapable_ids = std::vector<std::vector<int>>(p.patients.size());
    for (int i = 0; i < p.patients.size(); i++) {
        for (int j : p.patient_kickable_ids[i]) {
            if (i > j) {
                continue;
            }

            p.patient_swapable_ids[i].push_back(j);
        }
    }

    for (int i = 0; i < p.patients.size(); i++) {
        std::shuffle(p.patient_allowed_value_idxs_list[i].begin(), p.patient_allowed_value_idxs_list[i].end(), gen);
        std::shuffle(p.patient_kickable_ids[i].begin(), p.patient_kickable_ids[i].end(), gen);
        std::shuffle(p.patient_kickableout_ids[i].begin(), p.patient_kickableout_ids[i].end(), gen);
        std::shuffle(p.patient_swapable_ids[i].begin(), p.patient_swapable_ids[i].end(), gen);
    }
    for (int day = 0; day < p.days; day++) {
        for (int i = 0; i < p.num_shift_types; i++) {
            std::shuffle(p.day_shift_nurse_ids[day][i].begin(), p.day_shift_nurse_ids[day][i].end(), gen);
        }
    }


    return p;
}

void Problem::print_short_info() const {
    std::cout << "Instance: " << filename << std::endl;
    std::cout << "Number of patients: " << patients.size() << std::endl;
    std::cout << "Number of occupants: " << occupants.size() << std::endl;
    std::cout << "Number of rooms: " << rooms.size() << std::endl;
    std::cout << "Number of operating theaters: " << operating_theaters.size() << std::endl;
    std::cout << "Number of nurses: " << nurses.size() << std::endl;
    std::cout << "Number of surgeons: " << surgeons.size() << std::endl;
    std::cout << "Number of days: " << days << std::endl;
    std::cout << "Number of shift types: " << num_shift_types << ", age groups: " << num_age_groups << ", skill levels: " << num_skill_levels << std::endl;
}
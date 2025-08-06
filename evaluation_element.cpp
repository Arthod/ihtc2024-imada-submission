#include "evaluation_element.h"
#include "solution_data.h"
#include "config.h"


int EvalElemSoftConstraints::get_objective(SolutionData& sd, bool is_debug) {
    // EvalElemOpenOperatingTheater
    // EvalElemRoomAgeMix
    // EvalElemPatientDelay
    // EvalElemElectiveUnscheduledPatients
    // EvalElemSurgeonTransfer
    // EvalElemRoomSkillLevel
    // EvalElemContinuityOfCare
    // EvalElemExcessiveNurseWorkload
    
    std::vector<std::vector<int>> operating_theater_day_loads(problem.operating_theaters.size(), std::vector<int>(problem.days, 0));
    std::vector<std::vector<std::vector<int>>> room_day_ages = std::vector<std::vector<std::vector<int>>>(problem.rooms.size(), std::vector<std::vector<int>>(problem.days, std::vector<int>(problem.num_age_groups, 0)));
    std::vector<std::vector<int>> room_day_agegroup_max(problem.rooms.size(), std::vector<int>(problem.days, 0));
    std::vector<std::vector<int>> room_day_agegroup_min(problem.rooms.size(), std::vector<int>(problem.days, 0));
    std::vector<std::vector<std::vector<int>>> day_surgeon_operating_theater_loads(problem.days, std::vector<std::vector<int>>(problem.surgeons.size(), std::vector<int>(problem.operating_theaters.size(), 0)));
    std::vector<std::vector<std::vector<std::vector<int>>>> room_day_shift_skill_level_patients(problem.rooms.size(), std::vector<std::vector<std::vector<int>>>(problem.days, std::vector<std::vector<int>>(problem.num_shift_types, std::vector<int>(problem.num_skill_levels, 0))));
    std::vector<int> patient_nurse_assignments((problem.patients.size() + problem.occupants.size()) * problem.nurses.size(), 0);
    std::vector<std::vector<int>> room_day_patients(problem.rooms.size() * problem.days, std::vector<int>());
    std::vector<int> nurse_day_shift_workloads(problem.nurses.size() * problem.days * problem.num_shift_types, 0);
    std::vector<int> room_day_shift_patients_workload(problem.rooms.size() * problem.days * problem.num_shift_types, 0);

    for (int i = 0; i < problem.nurses.size(); i++) {
        const Nurse& nurse = problem.nurses[i];

        for (int j = 0; j < nurse.working_shifts.size(); j++) {
            const WorkingShift& shift = nurse.working_shifts[j];

            int idx = i * problem.days * problem.num_shift_types + shift.day * problem.num_shift_types + shift.shift_index;
            nurse_day_shift_workloads[idx] -= shift.max_load;
        }
    }

    // Add preoccupants
    for (int i = 0; i < problem.occupants.size(); i++) {
        const Occupant& occupant = problem.occupants[i];
        int room_id = occupant.room_index;

        int day_end = std::min(occupant.length_of_stay, problem.days);
        for (int day = 0; day < day_end; day++) {
            room_day_ages[room_id][day][occupant.age_group]++;
            int idx1 = (room_id * problem.days) + day;
            room_day_patients[idx1].push_back(i + problem.patients.size());

            for (int shift = 0; shift < problem.num_shift_types; shift++) {
                int shift_idx = day * problem.num_shift_types + shift;
                int skill_requirement = occupant.skill_level_required[shift_idx];
                room_day_shift_skill_level_patients[room_id][day][shift][skill_requirement]++;
                
                int idx_rdsn = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                int nurse_id = sd.room_day_shift_nurse[idx_rdsn];
                int idx2 = (i + problem.patients.size()) * problem.nurses.size() + nurse_id;
                patient_nurse_assignments[idx2]++;

                int workload = occupant.workload_produced[day * problem.num_shift_types + shift];
                int idx_nurse = nurse_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                int idx_room = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                nurse_day_shift_workloads[idx_nurse] += workload;
                room_day_shift_patients_workload[idx_room] += workload;
            }
        }
    }

    // Add patients
    int penalty_OpenOperatingTheater = 0;
    int penalty_RoomAgeMix = 0;
    int penalty_PatientDelay = 0;
    int penalty_ElectiveUnscheduledPatients = 0;
    int penalty_SurgeonTransfer = 0;
    int penalty_RoomSkillLevel = 0;
    int penalty_ContinuityOfCare = 0;
    int penalty_ExcessiveNurseWorkload = 0;
    for (const auto& patient_id : sd.patients_scheduled) {
        const Patient& patient = problem.patients[patient_id];
        int room_id = sd.patient_room_ids[patient_id];
        int admission_day = sd.patient_admission_days[patient_id];
        int operating_theater_id = sd.patient_operating_theaters[patient_id];
        int surgeon_id = patient.surgeon_index;
        
        int day_end = std::min(admission_day + patient.length_of_stay, problem.days);
        for (int day = admission_day; day < day_end; day++) {
            room_day_ages[room_id][day][patient.age_group]++;
            int idx1 = (room_id * problem.days) + day;
            room_day_patients[idx1].push_back(patient_id);

            for (int shift = 0; shift < problem.num_shift_types; shift++) {
                int shift_idx = day * problem.num_shift_types + shift;
                int skill_requirement_idx = (day - admission_day) * problem.num_shift_types + shift;
                int skill_requirement = patient.skill_level_required[skill_requirement_idx];
                room_day_shift_skill_level_patients[room_id][day][shift][skill_requirement]++;

                int idx_rdsn = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                int nurse_id = sd.room_day_shift_nurse[idx_rdsn];
                int idx2 = (patient_id * problem.nurses.size()) + nurse_id;
                patient_nurse_assignments[idx2]++;

                int workload = patient.workload_produced[skill_requirement_idx];
                int idx_nurse = nurse_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                int idx_room = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                nurse_day_shift_workloads[idx_nurse] += workload;
                room_day_shift_patients_workload[idx_room] += workload;
            }
        }
        if (admission_day > patient.surgery_release_day) {
            penalty_PatientDelay += admission_day - patient.surgery_release_day;
        }
        if (operating_theater_day_loads[operating_theater_id][admission_day] == 0) {
            penalty_OpenOperatingTheater++;
        }
        operating_theater_day_loads[operating_theater_id][admission_day]++;
        day_surgeon_operating_theater_loads[admission_day][surgeon_id][operating_theater_id] += 1;
    }
    for (int i = 0; i < problem.patients.size(); i++) {
        const Patient& patient = problem.patients[i];
        if (patient.mandatory) {
            continue;
        }
        if (sd.patient_room_ids[i] == -1) {
            penalty_ElectiveUnscheduledPatients++;
        }
    }
    for (int patient_id = 0; patient_id < problem.patients.size() + problem.occupants.size(); patient_id++) {
        int sum_assignments = 0;

        for (int nurse_id = 0; nurse_id < problem.nurses.size(); nurse_id++) {
            int idx = (patient_id * problem.nurses.size()) + nurse_id;
            if (patient_nurse_assignments[idx] > 0) {
                sum_assignments++;
            }
        }

        penalty_ContinuityOfCare += sum_assignments;
    }
    
    for (int room_id = 0; room_id < problem.rooms.size(); room_id++) {
        for (int day = 0; day < problem.days; day++) {
            auto& ages = room_day_ages[room_id][day];

            int num_patients = 0;
            int age_group_max = 0;
            int age_group_min = sd.problem.num_age_groups - 1;
            for (int a = 0; a < sd.problem.num_age_groups; a++) {
                if (ages[a] > 0) {
                    if (age_group_max < a) {
                        age_group_max = a;
                    }
                    if (age_group_min > a) {
                        age_group_min = a;
                    }
                    num_patients += ages[a];
                }
            }
            if (num_patients == 0) {
                age_group_max = -1;
                age_group_min = -1;
            }

            room_day_agegroup_max[room_id][day] = age_group_max;
            room_day_agegroup_min[room_id][day] = age_group_min;
            penalty_RoomAgeMix += age_group_max - age_group_min;

            for (int shift = 0; shift < problem.num_shift_types; shift++) {
                int idx = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                int nurse_id = sd.room_day_shift_nurse[idx];
                int nurse_skill = problem.nurses[nurse_id].skill_level;

                // Loop in reverse
                for (int k = problem.num_skill_levels - 1; k >= 0; k--) {
                    if (nurse_skill >= k) {
                        break;
                    }

                    penalty_RoomSkillLevel += (k - nurse_skill) * room_day_shift_skill_level_patients[room_id][day][shift][k];
                }
            }
        }
    }
    for (int day = 0; day < problem.days; day++) {
        for (const auto& operating_theater_load : day_surgeon_operating_theater_loads[day]) {
            int num_load = 0;
            for (int load : operating_theater_load) {
                if (load > 0) {
                    num_load++;
                }
            }

            if (num_load >= 2) {
                penalty_SurgeonTransfer += num_load - 1;
            }
        }
    }
    for (int i = 0; i < problem.nurses.size(); i++) {
        const Nurse& nurse = problem.nurses[i];

        for (int j = 0; j < nurse.working_shifts.size(); j++) {
            const WorkingShift& wshift = nurse.working_shifts[j];
            int day = wshift.day;
            int shift = wshift.shift_index;

            int idx = i * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
            if (nurse_day_shift_workloads[idx] > 0) {
                penalty_ExcessiveNurseWorkload += nurse_day_shift_workloads[idx];
            }
        }
    }


    if (!is_debug) {
        sd.operating_theater_day_loads2 = operating_theater_day_loads;
        sd.room_day_ages = room_day_ages;
        sd.room_day_agegroup_max = room_day_agegroup_max;
        sd.room_day_agegroup_min = room_day_agegroup_min;
        sd.day_surgeon_operating_theater_loads = day_surgeon_operating_theater_loads;
        sd.room_day_shift_skill_level_patients = room_day_shift_skill_level_patients;
        sd.patient_nurse_assignments = patient_nurse_assignments;
        sd.room_day_patients = room_day_patients;
        sd.nurse_day_shift_workloads = nurse_day_shift_workloads;
        sd.room_day_shift_patients_workload = room_day_shift_patients_workload;
    }

    return penalty_OpenOperatingTheater * weight_OpenOperatingTheater
        + penalty_RoomAgeMix * weight_RoomAgeMix
        + penalty_PatientDelay * weight_PatientDelay
        + penalty_ElectiveUnscheduledPatients * weight_ElectiveUnscheduledPatients
        + penalty_SurgeonTransfer * weight_SurgeonTransfer
        + penalty_RoomSkillLevel * weight_RoomSkillLevel
        + penalty_ContinuityOfCare * weight_ContinuityOfCare
        + penalty_ExcessiveNurseWorkload * weight_ExcessiveNurseWorkload
       ;
}


int EvalElemSoftConstraints::get_incr(SolutionData& sd, MoveSetPatient& move, bool is_commit) {
    // EvalElemOpenOperatingTheater
    // EvalElemRoomAgeMix
    // EvalElemPatientDelay
    // EvalElemElectiveUnscheduledPatients
    // EvalElemSurgeonTransfer
    // EvalElemRoomSkillLevel
    // EvalElemContinuityOfCare
    // EvalElemExcessiveNurseWorkload

    bool day_same = sd.patient_admission_days[move.patient_id] == move.new_admission_day;
    bool day_and_ot_same = day_same && sd.patient_operating_theaters[move.patient_id] == move.new_operating_theater_id;
    bool day_and_room_same = day_same && sd.patient_room_ids[move.patient_id] == move.new_room_id;

    auto& operating_theater_day_loads = sd.operating_theater_day_loads2;
    auto& room_day_ages = sd.room_day_ages;
    auto& day_surgeon_operating_theater_loads = sd.day_surgeon_operating_theater_loads;
    auto& room_day_shift_skill_level_patients = sd.room_day_shift_skill_level_patients;
    auto& patient_nurse_assignments = sd.patient_nurse_assignments;
    auto& room_day_patients = sd.room_day_patients;
    auto& nurse_day_shift_workloads = sd.nurse_day_shift_workloads;
    auto& room_day_shift_patients_workload = sd.room_day_shift_patients_workload;

    const Patient& patient = problem.patients[move.patient_id];
    int age_group = patient.age_group;
    int surgeon_id = patient.surgeon_index;

    int incr_OpenOperatingTheater = 0;
    int incr_RoomAgeMix = 0;
    int incr_PatientDelay = 0;
    int incr_ElectiveUnscheduledPatients = 0;
    int incr_SurgeonTransfer = 0;
    int incr_RoomSkillLevel = 0;
    int incr_ContinuityOfCare = 0;
    int incr_ExcessiveNurseWorkload = 0;

    // Subtract old penalty
    if (sd.patient_operating_theaters[move.patient_id] != -1) {
        int admission_day = sd.patient_admission_days[move.patient_id];
        int operating_theater_id = sd.patient_operating_theaters[move.patient_id];
        int room_id = sd.patient_room_ids[move.patient_id];

        if (!day_and_ot_same) {
            if (operating_theater_day_loads[operating_theater_id][admission_day] == 1) {
                incr_OpenOperatingTheater -= 1;
            }

            if (is_commit) {
                operating_theater_day_loads[operating_theater_id][admission_day] -= 1;
            }

            int theaters_num = 0;
            for (int load : day_surgeon_operating_theater_loads[admission_day][surgeon_id]) {
                if (load > 0) {
                    theaters_num++;
                }
            }
    
            if (day_surgeon_operating_theater_loads[admission_day][surgeon_id][operating_theater_id] == 1 && theaters_num >= 2) {
                incr_SurgeonTransfer -= 1;
            }
    
            day_surgeon_operating_theater_loads[admission_day][surgeon_id][operating_theater_id] -= 1;
        }

        if (!day_and_room_same) {
            int day_end = std::min(admission_day + patient.length_of_stay, problem.days);
            for (int day = admission_day; day < day_end; day++) {
                for (int shift = 0; shift < problem.num_shift_types; shift++) {
                    int shift_idx = day * problem.num_shift_types + shift;
                    int idx = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                    int nurse_id = sd.room_day_shift_nurse[idx];
                    int workload_idx = (day - admission_day) * problem.num_shift_types + shift;
                    int workload = patient.workload_produced[workload_idx];
                    
                    int idx1 = nurse_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                    if (nurse_day_shift_workloads[idx1] - workload >= 0) {
                        incr_ExcessiveNurseWorkload -= workload;
                    } else if (nurse_day_shift_workloads[idx1] > 0) {
                        incr_ExcessiveNurseWorkload -= nurse_day_shift_workloads[idx1];
                    }

                    int skill_requirement = patient.skill_level_required[workload_idx];
    
                    int nurse_skill = problem.nurses[nurse_id].skill_level;
    
                    if (nurse_skill < skill_requirement) {
                        incr_RoomSkillLevel -= skill_requirement - nurse_skill;
                    }
    
                    if (is_commit) {
                        int idx2 = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                        nurse_day_shift_workloads[idx1] -= workload;
                        room_day_shift_patients_workload[idx2] -= workload;
                        room_day_shift_skill_level_patients[room_id][day][shift][skill_requirement]--;
                    }
                }

                if (room_id == move.new_room_id && day >= move.new_admission_day && day < move.new_admission_day + patient.length_of_stay) {
                    continue;
                }

                auto& ages = room_day_ages[room_id][day];
                int age_group_max = sd.room_day_agegroup_max[room_id][day];
                int age_group_min = sd.room_day_agegroup_min[room_id][day];

                if (ages[age_group] == 1) {
                    if (age_group == age_group_max) {
                        // Removed patient was the oldest and only patient at that age
                        // Need to find new max 
                        for (int a = age_group_max - 1; a >= age_group_min; a--) {
                            if (ages[a] > 0) {
                                incr_RoomAgeMix -= age_group_max - a;
                                age_group_max = a;
                                break;
                            }
                        }
                    } else if (age_group == age_group_min) {
                        // Removed patient was the youngest and only patient at that age
                        // Need to find new min
                        for (int a = age_group_min + 1; a <= age_group_max; a++) {
                            if (ages[a] > 0) {
                                incr_RoomAgeMix -= a - age_group_min;
                                age_group_min = a;
                                break;
                            }
                        }
                    }
                }

                for (int shift = 0; shift < problem.num_shift_types; shift++) {
                    int shift_idx = day * problem.num_shift_types + shift;
                    int idx = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                    int nurse_id = sd.room_day_shift_nurse[idx];

                    int patient_nurse_idx = (move.patient_id * problem.nurses.size()) + nurse_id;
                    if (patient_nurse_assignments[patient_nurse_idx] + delta[nurse_id] == 1) {
                        incr_ContinuityOfCare -= 1;
                    }
    
                    if (is_commit) {
                        patient_nurse_assignments[patient_nurse_idx]--;
                    } else {
                        delta[nurse_id]--;
                    }
                }

                if (is_commit) {
                    ages[age_group] -= 1;
                    sd.room_day_agegroup_max[room_id][day] = age_group_max;
                    sd.room_day_agegroup_min[room_id][day] = age_group_min;

                    auto& patients = room_day_patients[(room_id * problem.days) + day];
                    patients.erase(std::remove(patients.begin(), patients.end(), move.patient_id), patients.end());
                }
            }
        }

        if (!day_same) {
            if (admission_day > patient.surgery_release_day) {
                incr_PatientDelay -= admission_day - patient.surgery_release_day;
            }
        }
    } else {
        incr_ElectiveUnscheduledPatients -= 1;
    }

    // Add new penalty
    if (move.new_room_id != -1) {
        int admission_day = move.new_admission_day;
        int operating_theater_id = move.new_operating_theater_id;
        int room_id = move.new_room_id;
        bool is_overlap1 = !is_commit && sd.patient_room_ids[move.patient_id] != -1;

        if (!day_and_ot_same) {
            if (operating_theater_day_loads[operating_theater_id][admission_day] == 0) {
                incr_OpenOperatingTheater += 1;
            }

            int theaters_num = 0;
            for (int load : day_surgeon_operating_theater_loads[admission_day][surgeon_id]) {
                if (load > 0) {
                    theaters_num++;
                }
            }
    
            if (day_surgeon_operating_theater_loads[admission_day][surgeon_id][operating_theater_id] == 0 && theaters_num >= 1) {
                incr_SurgeonTransfer += 1;
            }
    
            if (is_commit) {
                day_surgeon_operating_theater_loads[admission_day][surgeon_id][operating_theater_id] += 1;
                operating_theater_day_loads[operating_theater_id][admission_day] += 1;
            }
        }

        if (!day_and_room_same) {
            int day_end = std::min(admission_day + patient.length_of_stay, problem.days);
            for (int day = admission_day; day < day_end; day++) {
                bool is_overlap2 = is_overlap1 && sd.patient_admission_days[move.patient_id] <= day && day < sd.patient_admission_days[move.patient_id] + patient.length_of_stay;
                for (int shift = 0; shift < problem.num_shift_types; shift++) {
                    int shift_idx = day * problem.num_shift_types + shift;
                    int idx = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                    int nurse_id = sd.room_day_shift_nurse[idx];
                    int workload_idx = (day - admission_day) * problem.num_shift_types + shift;
                    int workload = patient.workload_produced[workload_idx];
    
                    int workload_other = 0;
                    int idx_room_other = sd.patient_room_ids[move.patient_id] * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                    if (is_overlap2 && sd.room_day_shift_nurse[idx_room_other] == nurse_id) {
                        workload_other = patient.workload_produced[(day - sd.patient_admission_days[move.patient_id]) * problem.num_shift_types + shift];
                    }
                    
                    int idx1 = nurse_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                    if (nurse_day_shift_workloads[idx1] - workload_other >= 0) {
                        incr_ExcessiveNurseWorkload += workload;
                    } else if (nurse_day_shift_workloads[idx1] + workload > workload_other) {
                        incr_ExcessiveNurseWorkload += nurse_day_shift_workloads[idx1] + workload - workload_other;
                    }

                    int skill_requirement = patient.skill_level_required[workload_idx];
                    int nurse_skill = problem.nurses[nurse_id].skill_level;
    
                    if (nurse_skill < skill_requirement) {
                        incr_RoomSkillLevel += skill_requirement - nurse_skill;
                    }
    
                    if (is_commit) {
                        int idx2 = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                        nurse_day_shift_workloads[idx1] += workload;
                        room_day_shift_patients_workload[idx2] += workload;
                        room_day_shift_skill_level_patients[room_id][day][shift][skill_requirement]++;
                    }
                }

                if (room_id == sd.patient_room_ids[move.patient_id] && day >= sd.patient_admission_days[move.patient_id] && day < sd.patient_admission_days[move.patient_id] + patient.length_of_stay) {
                    continue;
                }
                auto& ages = room_day_ages[room_id][day];
                int age_group_max = sd.room_day_agegroup_max[room_id][day];
                int age_group_min = sd.room_day_agegroup_min[room_id][day];
    
                if (ages[age_group_max] == 0) {
                    age_group_max = age_group;
                    age_group_min = age_group;
                } else if (age_group > age_group_max) { // New max
                    incr_RoomAgeMix += age_group - age_group_max;
                    age_group_max = age_group;
                } else if (age_group < age_group_min) { // New min
                    incr_RoomAgeMix += age_group_min - age_group;
                    age_group_min = age_group;
                }

                for (int shift = 0; shift < problem.num_shift_types; shift++) {
                    int shift_idx = day * problem.num_shift_types + shift;
                    int idx = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                    int nurse_id = sd.room_day_shift_nurse[idx];

                    int patient_nurse_idx = (move.patient_id * problem.nurses.size()) + nurse_id;
                    if (patient_nurse_assignments[patient_nurse_idx] + delta[nurse_id] == 0) {
                        incr_ContinuityOfCare += 1;
                    }
    
                    if (is_commit) {
                        patient_nurse_assignments[patient_nurse_idx]++;
                    } else {
                        delta[nurse_id]++;
                    }
                }
    
                if (is_commit) {
                    ages[age_group] += 1;
                    sd.room_day_agegroup_max[room_id][day] = age_group_max;
                    sd.room_day_agegroup_min[room_id][day] = age_group_min;

                    room_day_patients[(room_id * problem.days) + day].push_back(move.patient_id);
                }
            }
        }

        if (!day_same) {
            if (admission_day > patient.surgery_release_day) {
                incr_PatientDelay += admission_day - patient.surgery_release_day;
            }
        }
    } else {
        incr_ElectiveUnscheduledPatients += 1;
    }

    if (!is_commit) {
        if (!day_and_ot_same && sd.patient_operating_theaters[move.patient_id] != -1) {
            int admission_day = sd.patient_admission_days[move.patient_id];
            int operating_theater_id = sd.patient_operating_theaters[move.patient_id];

            day_surgeon_operating_theater_loads[admission_day][surgeon_id][operating_theater_id] += 1;
        }
        if (!day_and_room_same) {
            delta.assign(problem.nurses.size(), 0);
        }
    }

    return incr_OpenOperatingTheater * weight_OpenOperatingTheater
        + incr_RoomAgeMix * weight_RoomAgeMix
        + incr_PatientDelay * weight_PatientDelay
        + incr_ElectiveUnscheduledPatients * weight_ElectiveUnscheduledPatients
        + incr_SurgeonTransfer * weight_SurgeonTransfer
        + incr_RoomSkillLevel * weight_RoomSkillLevel
        + incr_ContinuityOfCare * weight_ContinuityOfCare
        + incr_ExcessiveNurseWorkload * weight_ExcessiveNurseWorkload
       ;
}

int EvalElemSoftConstraints::get_incr(SolutionData& sd, MoveSetNurse& move, bool is_commit) {
    // EvalElemOpenOperatingTheater
    // EvalElemRoomAgeMix
    // EvalElemPatientDelay
    // EvalElemElectiveUnscheduledPatients
    // EvalElemSurgeonTransfer
    // EvalElemRoomSkillLevel
    // EvalElemContinuityOfCare
    // EvalElemExcessiveNurseWorkload

    int idx = move.room_id * problem.days * problem.num_shift_types + move.day * problem.num_shift_types + move.shift;
    int nurse_id_old = sd.room_day_shift_nurse[idx];
    if (nurse_id_old == move.nurse_id) {
        return 0;
    }

    auto& room_day_shift_skill_level_patients = sd.room_day_shift_skill_level_patients;
    auto& patient_nurse_assignments = sd.patient_nurse_assignments;
    auto& room_day_patients = sd.room_day_patients;
    auto& nurse_day_shift_workloads = sd.nurse_day_shift_workloads;
    auto& room_day_shift_patients_workload = sd.room_day_shift_patients_workload;

    int incr_RoomSkillLevel = 0;
    int incr_ContinuityOfCare = 0;
    int incr_ExcessiveNurseWorkload = 0;

    int nurse_skill_old = problem.nurses[nurse_id_old].skill_level;
    int nurse_skill_new = problem.nurses[move.nurse_id].skill_level;
    if (nurse_skill_old != nurse_skill_new) {
        for (int i = 0; i < room_day_shift_skill_level_patients[move.room_id][move.day][move.shift].size(); i++) {
            if (nurse_skill_old < i) {
                incr_RoomSkillLevel -= (i - nurse_skill_old) * room_day_shift_skill_level_patients[move.room_id][move.day][move.shift][i];
            }

            if (nurse_skill_new < i) {
                incr_RoomSkillLevel += (i - nurse_skill_new) * room_day_shift_skill_level_patients[move.room_id][move.day][move.shift][i];
            }
        }
    }

    int room_id = move.room_id;
    int idx1 = (room_id * problem.days) + move.day;
    for (int patient_id : room_day_patients[idx1]) {
        int idx2_old = (patient_id * problem.nurses.size()) + nurse_id_old;
        int idx2_new = (patient_id * problem.nurses.size()) + move.nurse_id;
        // Subtract old penalty
        if (patient_nurse_assignments[idx2_old] == 1) {
            incr_ContinuityOfCare -= 1;
        }

        // Add new penalty
        if (patient_nurse_assignments[idx2_new] == 0) {
            incr_ContinuityOfCare += 1;
        }

        if (is_commit) {
            patient_nurse_assignments[idx2_old]--;
            patient_nurse_assignments[idx2_new]++;
        }
    }

    // Subtract old penalty
    int workload_from_patients = room_day_shift_patients_workload[idx];
    int idx_nurse_old = nurse_id_old * problem.days * problem.num_shift_types + move.day * problem.num_shift_types + move.shift;
    int workload1 = nurse_day_shift_workloads[idx_nurse_old];
    if (workload1 > 0) {
        if (workload1 - workload_from_patients >= 0) {
            incr_ExcessiveNurseWorkload -= workload_from_patients;
        } else {
            incr_ExcessiveNurseWorkload -= workload1;
        }
    }
    if (is_commit) {
        nurse_day_shift_workloads[idx_nurse_old] -= workload_from_patients;
    }

    // Add new penalty
    int idx_nurse_new = move.nurse_id * problem.days * problem.num_shift_types + move.day * problem.num_shift_types + move.shift;
    int workload2 = nurse_day_shift_workloads[idx_nurse_new];
    if (workload2 >= 0) {
        incr_ExcessiveNurseWorkload += workload_from_patients;
    } else if (workload2 + workload_from_patients > 0) {
        incr_ExcessiveNurseWorkload += workload2 + workload_from_patients;
    }

    if (is_commit) {
        nurse_day_shift_workloads[idx_nurse_new] += workload_from_patients;
    }

    return incr_RoomSkillLevel * weight_RoomSkillLevel
        + incr_ContinuityOfCare * weight_ContinuityOfCare
        + incr_ExcessiveNurseWorkload * weight_ExcessiveNurseWorkload
       ;
}





















int EvalElemHardConstraints::get_objective(SolutionData& sd, bool is_debug) {
    std::vector<std::array<int, 2>> room_day_genders(problem.rooms.size() * problem.days, {0, 0});
    std::vector<std::vector<int>> surgeon_day_loads(problem.surgeons.size(), std::vector<int>(problem.days, 0));
    std::vector<std::vector<int>> operating_theater_day_loads(problem.operating_theaters.size(), std::vector<int>(problem.days, 0));
    std::vector<int> room_day_loads(problem.rooms.size() * problem.days, 0);

    // Add preoccupants
    for (int i = 0; i < problem.occupants.size(); i++) {
        const Occupant& occupant = problem.occupants[i];

        for (int day = 0; day < occupant.length_of_stay; day++) {
            int idx = occupant.room_index * problem.days + day;
            room_day_genders[idx][occupant.gender]++;
            room_day_loads[idx]++;
        }
    }

    // Add patients
    int penalty = 0;
    for (const auto& patient_id : sd.patients_scheduled) {
        const Patient& patient = problem.patients[patient_id];
        
        int room_id = sd.patient_room_ids[patient_id];
        int admission_day = sd.patient_admission_days[patient_id];
        int operating_theater_id = sd.patient_operating_theaters[patient_id];
        
        int added = patient.surgery_duration;
        
        int day_end = std::min(admission_day + patient.length_of_stay, problem.days);
        for (int day = admission_day; day < day_end; day++) {
            int idx = room_id * problem.days + day;

            room_day_genders[idx][patient.gender]++;
            penalty += room_day_genders[idx][1 - patient.gender];

            if (room_day_loads[idx] >= problem.rooms[room_id].capacity) {
                penalty++;
            }
            room_day_loads[idx]++;
        }

        int surgeon_id = patient.surgeon_index;
        int surgeon_capacity = problem.surgeons[surgeon_id].max_surgery_time[admission_day];
        int surgeon_current = surgeon_day_loads[surgeon_id][admission_day];
        if (surgeon_current > surgeon_capacity) {
            penalty += added;
        } else if (surgeon_current + added > surgeon_capacity) {
            penalty += surgeon_current + added - surgeon_capacity;
        }
        surgeon_day_loads[surgeon_id][admission_day] += added;


        int ot_capacity = problem.operating_theaters[operating_theater_id].availability[admission_day];
        int ot_current = operating_theater_day_loads[operating_theater_id][admission_day];
        if (ot_current > ot_capacity) {
            penalty += added;
        } else if (ot_current + added > ot_capacity) {
            penalty += ot_current + added - ot_capacity;
        }
        operating_theater_day_loads[operating_theater_id][admission_day] += added;
    }

    if (!is_debug) {
        sd.room_day_genders = room_day_genders;
        sd.surgeon_day_loads = surgeon_day_loads;
        sd.operating_theater_day_loads = operating_theater_day_loads;
        sd.room_day_loads = room_day_loads;
    }

    return penalty;
}

int EvalElemHardConstraints::get_incr(SolutionData& sd, MoveSetPatient& move, bool is_commit) {
    const Patient& patient = problem.patients[move.patient_id];

    bool day_same = move.new_admission_day == sd.patient_admission_days[move.patient_id];
    bool day_and_ot_same = day_same && move.new_operating_theater_id == sd.patient_operating_theaters[move.patient_id];
    bool day_and_room_same = day_same && move.new_room_id == sd.patient_room_ids[move.patient_id];

    auto& room_day_genders = sd.room_day_genders;
    auto& surgeon_day_loads = sd.surgeon_day_loads;
    auto& operating_theater_day_loads = sd.operating_theater_day_loads;
    auto& room_day_loads = sd.room_day_loads;

    int incr = 0;

    int surgeon_id = patient.surgeon_index;
    
    // Subtract old penalty
    if (sd.patient_room_ids[move.patient_id] != -1) {
        int admission_day = sd.patient_admission_days[move.patient_id];
        int room_id = sd.patient_room_ids[move.patient_id];
        int operating_theater_id = sd.patient_operating_theaters[move.patient_id];
        int removed = patient.surgery_duration;

        if (!day_and_room_same) {
            int day_end = std::min(admission_day + patient.length_of_stay, problem.days);
            for (int day = admission_day; day < day_end; day++) {
                int idx = room_id * problem.days + day;
                incr -= room_day_genders[idx][1 - patient.gender];

                if (is_commit) {
                    room_day_genders[idx][patient.gender] -= 1;
                }

                if (room_id == move.new_room_id && day >= move.new_admission_day && day < move.new_admission_day + patient.length_of_stay) {
                    continue;
                }

                if (room_day_loads[idx] > problem.rooms[room_id].capacity) {
                    incr -= 1;
                }
                if (is_commit) {
                    room_day_loads[idx] -= 1;
                }
            }
        }

        if (!day_same) {
            int surgeon_capacity = problem.surgeons[surgeon_id].max_surgery_time[admission_day];
            int surgeon_current = surgeon_day_loads[surgeon_id][admission_day];
            if (surgeon_current - removed > surgeon_capacity) {
                incr -= removed;
            } else if (surgeon_current > surgeon_capacity) {
                incr -= surgeon_current - surgeon_capacity;
            }

            if (is_commit) {
                surgeon_day_loads[surgeon_id][admission_day] -= removed;
            }
        }

        if (!day_and_ot_same) {
            int ot_capacity = problem.operating_theaters[operating_theater_id].availability[admission_day];
            int ot_current = operating_theater_day_loads[operating_theater_id][admission_day];
            if (ot_current - removed > ot_capacity) {
                incr -= removed;
            } else if (ot_current > ot_capacity) {
                incr -= ot_current - ot_capacity;
            }

            if (is_commit) {
                operating_theater_day_loads[operating_theater_id][admission_day] -= removed;
            }
        }
    }

    // Add new penalty
    if (move.new_room_id != -1) {
        int admission_day = move.new_admission_day;
        int room_id = move.new_room_id;
        int operating_theater_id = move.new_operating_theater_id;
        int added = patient.surgery_duration;

        if (!day_and_room_same) {
            int day_end = std::min(admission_day + patient.length_of_stay, problem.days);
            for (int day = admission_day; day < day_end; day++) {
                int idx = room_id * problem.days + day;
                incr += room_day_genders[idx][1 - patient.gender];

                if (is_commit) {
                    room_day_genders[idx][patient.gender] += 1;
                }

                if (room_id == sd.patient_room_ids[move.patient_id] && day >= sd.patient_admission_days[move.patient_id] && day < sd.patient_admission_days[move.patient_id] + patient.length_of_stay) {
                    continue;
                }
                
                if (room_day_loads[idx] >= problem.rooms[room_id].capacity) {
                    incr += 1;
                }
                if (is_commit) {
                    room_day_loads[idx] += 1;
                }
            }
        }

        if (!day_same) {
            int surgeon_capacity = problem.surgeons[surgeon_id].max_surgery_time[admission_day];
            int surgeon_current = surgeon_day_loads[surgeon_id][admission_day];
            if (surgeon_current > surgeon_capacity) {
                incr += added;
            } else if (surgeon_current + added > surgeon_capacity) {
                incr += surgeon_current + added - surgeon_capacity;
            }

            if (is_commit) {
                surgeon_day_loads[surgeon_id][admission_day] += added;
            }
        }

        if (!day_and_ot_same) {
            int ot_capacity = problem.operating_theaters[operating_theater_id].availability[admission_day];
            int ot_current = operating_theater_day_loads[operating_theater_id][admission_day];
            if (ot_current > ot_capacity) {
                incr += added;
            } else if (ot_current + added > ot_capacity) {
                incr += ot_current + added - ot_capacity;
            }

            if (is_commit) {
                operating_theater_day_loads[operating_theater_id][admission_day] += added;
            }
        }
    }

    return incr;
}
    
int EvalElemHardConstraints::get_incr(SolutionData& sd, MoveSetNurse& move, bool is_commit) {
    return 0;
}


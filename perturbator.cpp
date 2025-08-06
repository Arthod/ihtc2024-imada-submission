
#include "perturbator.h"
#include "move.h"
#include "evaluation.h"
#include "utils.h"
#include "move_generator.h"
#include "solution.h"
#include <math.h>
#include "config.h"
#include "algorithms.h"

PerturbatorUnion::PerturbatorUnion(const Problem& problem, std::mt19937& gen, std::vector<std::pair<Perturbator*, double>>& pu_weights) : problem(problem), gen(gen) {

    pus.reserve(pu_weights.size());
    weights.reserve(pu_weights.size());
    for (auto& pu_weight : pu_weights) {
        pus.push_back(std::unique_ptr<Perturbator>(pu_weight.first));
        weights.push_back(pu_weight.second);
    }

    compute_weight_indices();
    distr_perturbator_index = std::uniform_int_distribution<int>(0, perturbator_indices.size() - 1);
}

void PerturbatorUnion::compute_weight_indices() {
    perturbator_indices.clear();

    double weight_sum = 0;
    for (double weight : weights) {
        weight_sum += weight;
    }

    int size = 100000;
    for (int i = 0; i < pus.size(); i++) {
        int num = round((double) size * (weights[i] / (double) weight_sum));
        for (int j = 0; j < num; j++) {
            perturbator_indices.push_back(i);
        }
    }
}

int PerturbatorUnion::perturb(Solution& s, double ks) {
    int solution_changed = 0;
    int index = -1;
    while (solution_changed == 0) {
        index = perturbator_indices[distr_perturbator_index(gen)];
        solution_changed = pus[index]->perturb(s, ks);
    }
    return index;
}

int PerturbatorCloseOperatingTheaters::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    std::vector<int> operating_theaters_to_close_ids;
    std::vector<int> operating_theaters_open_ids;
    int solution_changed = 0;
    
    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int i = 0; i < problem.operating_theaters.size(); i++) {
        if (rand(gen) < ks2) {
            operating_theaters_open_ids.push_back(i);
        } else {
            operating_theaters_to_close_ids.push_back(i);
        }
    }
    if (operating_theaters_to_close_ids.size() == 0 || operating_theaters_open_ids.size() == 0) {
        return 0;
    }

    for (int i = 0; i < problem.patients.size(); i++) {
        int old_operating_theater_id = sd.patient_operating_theaters[i];
        if (old_operating_theater_id == -1) {
            continue;
        }

        if (std::find(operating_theaters_to_close_ids.begin(), operating_theaters_to_close_ids.end(), old_operating_theater_id) != operating_theaters_to_close_ids.end()) {
            int new_operating_theater_id = operating_theaters_open_ids[random_int(gen, 0, operating_theaters_open_ids.size() - 1)];
            int value_idx = sd.patient_room_ids[i] * problem.days * problem.operating_theaters.size() + sd.patient_admission_days[i] * problem.operating_theaters.size() + new_operating_theater_id;
            if (problem.patient_allowed_value_idxs_set[i].find(value_idx) == problem.patient_allowed_value_idxs_set[i].end()) {
                continue;
            }
            MoveSetPatient move = MoveSetPatient(i, sd.patient_room_ids[i], new_operating_theater_id, sd.patient_admission_days[i]);
            s.step(move);
            solution_changed = 1;
        }
    }

    return solution_changed;
}

int PerturbatorCloseRooms::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    std::vector<int> room_ids_to_close;
    std::vector<int> room_ids_open;
    int solution_changed = 0;
    
    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int i = 0; i < problem.rooms.size(); i++) {
        if (rand(gen) < ks2) {
            room_ids_open.push_back(i);
        } else {
            room_ids_to_close.push_back(i);
        }
    }
    if (room_ids_to_close.size() == 0 || room_ids_open.size() == 0) {
        return 0;
    }

    for (int i = 0; i < problem.patients.size(); i++) {
        int old_room_id = sd.patient_room_ids[i];
        if (old_room_id == -1) {
            continue;
        }

        if (std::find(room_ids_to_close.begin(), room_ids_to_close.end(), old_room_id) != room_ids_to_close.end()) {
            int new_room_id = room_ids_open[random_int(gen, 0, room_ids_open.size() - 1)];
            int value_idx = new_room_id * problem.days * problem.operating_theaters.size() + sd.patient_admission_days[i] * problem.operating_theaters.size() + sd.patient_operating_theaters[i];
            if (problem.patient_allowed_value_idxs_set[i].find(value_idx) == problem.patient_allowed_value_idxs_set[i].end()) {
                continue;
            }
            MoveSetPatient move = MoveSetPatient(i, new_room_id, sd.patient_operating_theaters[i], sd.patient_admission_days[i]);
            s.step(move);
            solution_changed = 1;
        }
    }

    return solution_changed;
}

int PerturbatorCloseOperatingTheaterDays::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    std::vector<int> operating_theater_days_to_close; // ot * len(days) + day
    std::vector<int> operating_theater_days_open;
    int solution_changed = 0;
    
    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int i = 0; i < problem.operating_theaters.size(); i++) {
        for (int j = 0; j < problem.days; j++) {
            int ot_day = i * problem.days + j;
            if (rand(gen) < ks2) {
                operating_theater_days_open.push_back(ot_day);
            } else {
                operating_theater_days_to_close.push_back(ot_day);
            }
        }
    }

    if (operating_theater_days_to_close.size() == 0 || operating_theater_days_open.size() == 0) {
        return 0;
    }

    for (int i = 0; i < problem.patients.size(); i++) {
        if (sd.patient_operating_theaters[i] == -1) {
            continue;
        }

        const Patient& patient = problem.patients[i];

        int old_ot_day = sd.patient_operating_theaters[i] * problem.days + sd.patient_admission_days[i];
        if (std::find(operating_theater_days_to_close.begin(), operating_theater_days_to_close.end(), old_ot_day) != operating_theater_days_to_close.end()) {
            int new_ot_day = operating_theater_days_open[random_int(gen, 0, operating_theater_days_open.size() - 1)];
            int new_operating_theater_id = new_ot_day / problem.days;
            int new_day = new_ot_day % problem.days;

            int value_idx = sd.patient_room_ids[i] * problem.days * problem.operating_theaters.size() + new_day * problem.operating_theaters.size() + new_operating_theater_id;
            if (std::find(problem.patient_allowed_value_idxs_set[i].begin(), problem.patient_allowed_value_idxs_set[i].end(), value_idx) == problem.patient_allowed_value_idxs_set[i].end()) {
                continue;
            }
            MoveSetPatient move = MoveSetPatient(i, sd.patient_room_ids[i], new_operating_theater_id, new_day);
            s.step(move);
            solution_changed = 1;
        }
    }

    return solution_changed;
}

int PerturbatorRemoveOptionalPatients::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    int solution_changed = 0;
    
    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int i = 0; i < problem.patients.size(); i++) {
        if (problem.patients[i].mandatory || sd.patient_room_ids[i] == -1) {
            continue;
        }

        if (rand(gen) >= ks2) {
            continue;
        }

        MoveSetPatient move = MoveSetPatient(i, -1, -1, -1);
        s.step(move);
        solution_changed = 1;
    }
    
    return solution_changed;
}

int PerturbatorFromGenerator::perturb(Solution& s, double ks) {
    int solution_changed = 0;

    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int i = 0; i < size * ks2; i++) {
        auto move = mg->next(s);
        if (move == nullptr) {
            continue;
        }

        s.step(*move);
        solution_changed = 1;
    }
    
    return solution_changed;
}

int PerturbatorEnforceAdmissionDay::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    int solution_changed = 0;
    
    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int i = 0; i < problem.patients.size(); i++) {
        if (rand(gen) >= ks2) {
            continue;
        }
        
        if (sd.patient_room_ids[i] == -1) {
            continue;
        }
        int new_admission_day = problem.patients[i].surgery_release_day;
        int new_operating_theater_id = sd.patient_operating_theaters[i];
        int new_room_id = sd.patient_room_ids[i];
        int value_idx = new_room_id * problem.days * problem.operating_theaters.size() + new_admission_day * problem.operating_theaters.size() + new_operating_theater_id;
        if (problem.patient_allowed_value_idxs_set[i].find(value_idx) == problem.patient_allowed_value_idxs_set[i].end()) {
            continue;
        }

        MoveSetPatient move = MoveSetPatient(i, new_room_id, new_operating_theater_id, new_admission_day);
        s.step(move);
        solution_changed = 1;
    }
    
    return solution_changed;
}

int PerturbatorPermutatePatientRooms::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    int solution_changed = 0;

    std::shuffle(room_idxs.begin(), room_idxs.end(), gen);

    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int i = 0; i < problem.patients.size(); i++) {
        if (sd.patient_room_ids[i] == -1) {
            continue;
        }
    
        if (rand(gen) >= ks2) {
            continue;
        }

        int new_room_id = room_idxs[sd.patient_room_ids[i]];

        int value_idx = new_room_id * problem.days * problem.operating_theaters.size() + sd.patient_admission_days[i] * problem.operating_theaters.size() + sd.patient_operating_theaters[i];
        if (problem.patient_allowed_value_idxs_set[i].find(value_idx) == problem.patient_allowed_value_idxs_set[i].end()) {
            continue;
        }
            
        MoveSetPatient move = MoveSetPatient(i, new_room_id, sd.patient_operating_theaters[i], sd.patient_admission_days[i]);
        s.step(move);
        solution_changed = 1;
    }
    
    return solution_changed;
}

int PerturbatorPermutatePatientOperatingTheaters::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    int solution_changed = 0;

    std::shuffle(operating_theater_idxs.begin(), operating_theater_idxs.end(), gen);

    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int i = 0; i < problem.patients.size(); i++) {
        if (sd.patient_operating_theaters[i] == -1) {
            continue;
        }
    
        if (rand(gen) >= ks2) {
            continue;
        }

        int new_operating_theater_id = operating_theater_idxs[sd.patient_operating_theaters[i]];
        int value_idx = sd.patient_room_ids[i] * problem.days * problem.operating_theaters.size() + sd.patient_admission_days[i] * problem.operating_theaters.size() + new_operating_theater_id;
        if (problem.patient_allowed_value_idxs_set[i].find(value_idx) == problem.patient_allowed_value_idxs_set[i].end()) {
            continue;
        }
        MoveSetPatient move = MoveSetPatient(i, sd.patient_room_ids[i], new_operating_theater_id, sd.patient_admission_days[i]);
        s.step(move);
        solution_changed = 1;
    }
    
    return solution_changed;
}

int PerturbatorChangeWeights::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    
    std::vector<int> weights_prev(8);
    double ks2 = ks * (ks_max - ks_min) + ks_min;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(1 - ks2, 1 + ks2);

    weights_prev[0] = eval_elem_soft_constraints.weight_ContinuityOfCare;
    weights_prev[1] = eval_elem_soft_constraints.weight_ElectiveUnscheduledPatients;
    weights_prev[2] = eval_elem_soft_constraints.weight_ExcessiveNurseWorkload;
    weights_prev[3] = eval_elem_soft_constraints.weight_OpenOperatingTheater;
    weights_prev[4] = eval_elem_soft_constraints.weight_PatientDelay;
    weights_prev[5] = eval_elem_soft_constraints.weight_RoomAgeMix;
    weights_prev[6] = eval_elem_soft_constraints.weight_RoomSkillLevel;
    weights_prev[7] = eval_elem_soft_constraints.weight_SurgeonTransfer;

    eval_elem_soft_constraints.weight_ContinuityOfCare = std::max(1, (int)((double) eval_elem_soft_constraints.weight_ContinuityOfCare * rand(gen)));
    eval_elem_soft_constraints.weight_ElectiveUnscheduledPatients = std::max(1, (int)((double) eval_elem_soft_constraints.weight_ElectiveUnscheduledPatients * rand(gen)));
    eval_elem_soft_constraints.weight_ExcessiveNurseWorkload = std::max(1, (int)((double) eval_elem_soft_constraints.weight_ExcessiveNurseWorkload * rand(gen)));
    eval_elem_soft_constraints.weight_OpenOperatingTheater = std::max(1, (int)((double) eval_elem_soft_constraints.weight_OpenOperatingTheater * rand(gen)));
    eval_elem_soft_constraints.weight_PatientDelay = std::max(1, (int)((double) eval_elem_soft_constraints.weight_PatientDelay * rand(gen)));
    eval_elem_soft_constraints.weight_RoomAgeMix = std::max(1, (int)((double) eval_elem_soft_constraints.weight_RoomAgeMix * rand(gen)));
    eval_elem_soft_constraints.weight_RoomSkillLevel = std::max(1, (int)((double) eval_elem_soft_constraints.weight_RoomSkillLevel * rand(gen)));
    eval_elem_soft_constraints.weight_SurgeonTransfer = std::max(1, (int)((double) eval_elem_soft_constraints.weight_SurgeonTransfer * rand(gen)));

    s.evaluation.values[last_index] = eval_elem_soft_constraints.get_objective(sd, false);

    //first_improvement(s, mg, 2, 0);
    fi_randomsubset_plus(s, eval_elem_hard_constraints, eval_elem_soft_constraints, mg, gen, 2, 1000, 0);
    //first_improvement_random_subset_neighborhood(s, mg, 2, 1000, 0);
    
    eval_elem_soft_constraints.weight_ContinuityOfCare = weights_prev[0];
    eval_elem_soft_constraints.weight_ElectiveUnscheduledPatients = weights_prev[1];
    eval_elem_soft_constraints.weight_ExcessiveNurseWorkload = weights_prev[2];
    eval_elem_soft_constraints.weight_OpenOperatingTheater = weights_prev[3];
    eval_elem_soft_constraints.weight_PatientDelay = weights_prev[4];
    eval_elem_soft_constraints.weight_RoomAgeMix = weights_prev[5];
    eval_elem_soft_constraints.weight_RoomSkillLevel = weights_prev[6];
    eval_elem_soft_constraints.weight_SurgeonTransfer = weights_prev[7];

    s.evaluation.values[last_index] = eval_elem_soft_constraints.get_objective(sd, false);

    return true;
}

int PerturbatorShuffleEquivalentNurses::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    int solution_changed = 0;

    std::vector<int> nurse_idxs;
    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int day = 0; day < problem.days; day++) {
        for (int room = 0; room < problem.rooms.size(); room++) {
            for (int shift = 0; shift < problem.num_shift_types; shift++) {
                if (rand(gen) >= ks2) {
                    continue;
                }

                int idx = room * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                int nurse_id = sd.room_day_shift_nurse[idx];
                const auto& nurse = problem.nurses[nurse_id];
                for (int nurse_id2 : problem.day_shift_nurse_ids[day][shift]) {
                    if (nurse_id == nurse_id2) {
                        continue;
                    }
                    const auto& nurse2 = problem.nurses[nurse_id2];
                    if (nurse.skill_level != nurse2.skill_level) {
                        continue;
                    }

                    MoveSetNurse move = MoveSetNurse(nurse_id2, room, day, shift);
                    s.step(move);
                    solution_changed = 1;
                    break;
                }
            }
        }       
    }
    
    return solution_changed;
}

int PerturbatorPatientAdmissionDayOffset::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    int solution_changed = 0;
    
    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int i = 0; i < problem.patients.size(); i++) {
        if (sd.patient_room_ids[i] == -1) {
            continue;
        }

        if (rand(gen) >= ks2) {
            continue;
        }

        int new_admission_day = sd.patient_admission_days[i] + offset;
        int day_end = problem.days;
        if (new_admission_day < 0 || new_admission_day >= day_end) {
            continue;
        }
        int value_idx = sd.patient_room_ids[i] * problem.days * problem.operating_theaters.size() + new_admission_day * problem.operating_theaters.size() + sd.patient_operating_theaters[i];
        if (std::find(problem.patient_allowed_value_idxs_set[i].begin(), problem.patient_allowed_value_idxs_set[i].end(), value_idx) == problem.patient_allowed_value_idxs_set[i].end()) {
            continue;
        }

        MoveSetPatient move = MoveSetPatient(i, sd.patient_room_ids[i], sd.patient_operating_theaters[i], new_admission_day);
        s.step(move);
        solution_changed = 1;
    }

    return solution_changed;
}

int PerturbatorPermutatePatientAdmissionDays::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    int solution_changed = 0;

    std::shuffle(admission_days.begin(), admission_days.end(), gen);

    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int i = 0; i < problem.patients.size(); i++) {
        if (sd.patient_room_ids[i] == -1) {
            continue;
        }
    
        if (rand(gen) >= ks2) {
            continue;
        }

        int new_admission_day = admission_days[sd.patient_admission_days[i]];
        int value_idx = sd.patient_room_ids[i] * problem.days * problem.operating_theaters.size() + new_admission_day * problem.operating_theaters.size() + sd.patient_operating_theaters[i];
        if (std::find(problem.patient_allowed_value_idxs_set[i].begin(), problem.patient_allowed_value_idxs_set[i].end(), value_idx) == problem.patient_allowed_value_idxs_set[i].end()) {
            continue;
        }

        MoveSetPatient move = MoveSetPatient(i, sd.patient_room_ids[i], sd.patient_operating_theaters[i], new_admission_day);
        s.step(move);
        solution_changed = 1;

    }

    return solution_changed;
}

int PerturbatorAssignUnscheduledPatientsBest::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    int solution_changed = 0;
    
    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int i = 0; i < problem.patients.size(); i++) {
        if (sd.patient_room_ids[i] != -1) {
            continue;
        }

        if (rand(gen) >= ks2) {
            continue;
        }

        int best_value_idx = -1;
        Evaluation best_incr = Evaluation({99999, 99999});
        for (int value_idx : problem.patient_allowed_value_idxs_list[i]) {
            const auto& value = problem.patient_values[value_idx];
            int room_id = value.room_id;
            int admission_day = value.admission_day;
            int operating_theater_id = value.operating_theater_id;

            MoveSetPatient move = MoveSetPatient(i, room_id, operating_theater_id, admission_day);
            auto incr = s.get_incr(sd, move, false);
            if (incr < best_incr) {
                best_value_idx = value_idx;
                best_incr = incr;
            }
        }
        if (best_value_idx == -1) {
            continue;
        }
        const auto& value = problem.patient_values[best_value_idx];
        MoveSetPatient move(i, value.room_id, value.operating_theater_id, value.admission_day);
        s.step(move);
        solution_changed = 1;
    }
    //std::cout << "Eval after: " << s.evaluation.to_string() << ", solution_changed: " << solution_changed << std::endl;

    return solution_changed;
}

int PerturbatorSurgeonDayRemoveSingleOT::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    int solution_changed = 0;

    double ks2 = ks * (ks_max - ks_min) + ks_min;
    std::vector<std::vector<std::vector<int>>> surgeon_day_ots(problem.surgeons.size(), std::vector<std::vector<int>>(problem.days, std::vector<int>(0, 0)));
    std::vector<std::vector<std::vector<std::vector<int>>>> surgeon_dan_ot_patients(problem.surgeons.size(), std::vector<std::vector<std::vector<int>>>(problem.days, std::vector<std::vector<int>>(problem.operating_theaters.size(), std::vector<int>(0, 0))));
    for (int patient_id : sd.patients_scheduled) {
        int surgeon_id = problem.patients[patient_id].surgeon_index;
        int day = sd.patient_admission_days[patient_id];
        int operating_theater_id = sd.patient_operating_theaters[patient_id];

        if (std::find(surgeon_day_ots[surgeon_id][day].begin(), surgeon_day_ots[surgeon_id][day].end(), operating_theater_id) == surgeon_day_ots[surgeon_id][day].end()) {
            surgeon_day_ots[surgeon_id][day].push_back(operating_theater_id);
        }

        surgeon_dan_ot_patients[surgeon_id][day][operating_theater_id].push_back(patient_id);
    }
    
    for (int surgeon = 0; surgeon < problem.surgeons.size(); surgeon++) {
        for (int i = 0; i < problem.days; i++) {
            if (rand(gen) >= ks2) {
                continue;
            }

            if (surgeon_day_ots[surgeon][i].size() <= 1) {
                continue;
            } 

            auto ot_distr = std::uniform_int_distribution<int>(0, surgeon_day_ots[surgeon][i].size() - 1);
            int ot_prev = surgeon_day_ots[surgeon][i][ot_distr(gen)];
            int ot_next = surgeon_day_ots[surgeon][i][ot_distr(gen)];
            while (ot_prev == ot_next) {
                ot_next = surgeon_day_ots[surgeon][i][ot_distr(gen)];
            }
            
            for (int j : surgeon_dan_ot_patients[surgeon][i][ot_prev]) {
                MoveSetPatient move(j, sd.patient_room_ids[j], ot_next, i);
                s.step(move);
                solution_changed = 1;
            }
        }
    }

    return solution_changed;
}

int PerturbatorNursePatientShuffleLowCareCount::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    int solution_changed = 0;

    //std::cout << "Eval before: " << s.evaluation.to_string() << std::endl;

    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int patient_id : sd.patients_scheduled) {
        if (rand(gen) >= ks2) {
            continue;
        }
        if (sd.patient_room_ids[patient_id] == -1) {
            continue;
        }

        const auto& patient = problem.patients[patient_id];
        int room_id = sd.patient_room_ids[patient_id];
        
        int admission_day = sd.patient_admission_days[patient_id];
        int day_end = std::min(problem.days, admission_day + patient.surgery_duration);
        for (int day = admission_day; day < day_end; day++) {
            for (int shift = 0; shift < problem.num_shift_types; shift++) {
                int idx_rdsn = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
                int nurse_id = sd.room_day_shift_nurse[idx_rdsn];
                
                int idx = patient_id * problem.nurses.size() + nurse_id;
                if (sd.patient_nurse_assignments[idx] > 1) {
                    continue;
                }

                auto nurse_id_distr = std::uniform_int_distribution<int>(0, problem.day_shift_nurse_ids[day][shift].size() - 1);
                int new_nurse_id = problem.day_shift_nurse_ids[day][shift][nurse_id_distr(gen)];
                if (new_nurse_id == nurse_id) {
                    continue;
                }

                MoveSetNurse move(new_nurse_id, room_id, day, shift);
                s.step(move);
                solution_changed = 1;
            }
        }
    }

    //std::cout << "Eval after: " << s.evaluation.to_string() << ", solution_changed: " << solution_changed << std::endl;

    return solution_changed;
}

int PerturbatorNurseShuffleOverWorkload::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);
    int solution_changed = 0;

    double ks2 = ks * (ks_max - ks_min) + ks_min;
    for (int nurse_id = 0; nurse_id < problem.nurses.size(); nurse_id++) {
        const Nurse& nurse = problem.nurses[nurse_id];

        for (const auto& working_shift : nurse.working_shifts) {
            if (rand(gen) >= ks2) {
                continue;
            }

            int day = working_shift.day;
            int shift = working_shift.shift_index;
            
            int idx = nurse_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
            if (sd.nurse_day_shift_workloads[idx] > working_shift.max_load) {
                for (int room : sd.nurse_day_shift_rooms[nurse_id][day][shift]) {
                    auto nurse_id_distr = std::uniform_int_distribution<int>(0, problem.day_shift_nurse_ids[day][shift].size() - 1);

                    int new_nurse_id = problem.day_shift_nurse_ids[day][shift][nurse_id_distr(gen)];
                    if (new_nurse_id == nurse_id) {
                        continue;
                    }

                    MoveSetNurse move(new_nurse_id, room, day, shift);
                    s.step(move);
                    solution_changed = 1;
                }
            }
        }
    }

    return solution_changed;
}

int PerturbatorSetPatientsMany::perturb(Solution& s, double ks) {
    auto& sd = s.sd;
    int solution_changed = 0;
    int num_patients = ks * (moves_max - moves_min) + moves_min;
    std::uniform_int_distribution<int> rand = std::uniform_int_distribution<int>(0, problem.patients.size() - 1);
    
    for (int i = 0; i < num_patients; i++) {
        int patient_idx = rand(gen);
        if (sd.patient_room_ids[patient_idx] == -1) {
            continue;
        }

        int patient_value_idx = problem.patient_allowed_value_idxs_list[patient_idx][random_int(gen, 0, problem.patient_allowed_value_idxs_list[patient_idx].size() - 1)];
        const auto& value = problem.patient_values[patient_value_idx];

        MoveSetPatient move(patient_idx, value.room_id, value.operating_theater_id, value.admission_day);
        s.step(move);
        solution_changed = 1;
    }

    return solution_changed;
}
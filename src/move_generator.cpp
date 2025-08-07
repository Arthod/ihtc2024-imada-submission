
#include "move_generator.h"
#include "move.h"
#include "iterable.h"
#include <coroutine>

#include "utils.h"
#include "problem.h"
#include "solution.h"
#include "config.h"

MoveGeneratorUnion::MoveGeneratorUnion(const Problem& problem, std::mt19937& gen, std::array<std::pair<MoveGenerator*, double>, 8>& mg_weights)
    : problem(problem), gen(gen), mg_weights(mg_weights) {
    compute_weight_indices();
    distr_mg_index = std::uniform_int_distribution<int>(0, 100000 - 1);
    for (int i = 0; i < mg_weights.size(); i++) {
        mgs[i] = mg_weights[i].first;
    }
}

void MoveGeneratorUnion::compute_weight_indices() {
    double weight_sum = 0;
    for (const auto& [_, weight] : mg_weights) {
        weight_sum += weight;
    }

    int size = 100000;
    int curr_index = 0;
    for (const auto& [mg, weight] : mg_weights) {
        int num = round((float) size * (weight / (float) weight_sum));
        for (int j = 0; j < num; j++) {
            if (curr_index >= 100000) {
                continue;
            }
            move_generator_indices[curr_index++] = mg;
        }
    }
    for (int i = curr_index; i < 100000; i++) {
        move_generator_indices[i] = move_generator_indices[curr_index - 1];
    }
}

Iterable<std::unique_ptr<Move>> MoveGeneratorUnion::random_moves(Solution& s) {
    for (int _ = 0; _ < mg_weights.size(); _++) {
        auto& mg = mgs[move_generator_index];
        auto iter = mg->random_moves(s);
        while (iter.move_next()) {
            auto move = iter.current();
            co_yield std::move(move);
        }

        move_generator_index = (move_generator_index + 1) % mg_weights.size();
    }

    co_return;
}

std::unique_ptr<Move> MoveGeneratorUnion::next(Solution& s) {
    auto& mg = move_generator_indices[distr_mg_index(gen)];
    return mg->next(s);
}

Iterable<std::unique_ptr<Move>> MoveGeneratorSetPatient::random_moves(Solution& s) {
    for (int _ = 0; _ < problem.patients.size(); _++) {
        const Patient& patient = problem.patients[patient_index];

        for (int patient_value_idx : problem.patient_allowed_value_idxs_list[patient_index]) {
            const auto& patient_value = problem.patient_values[patient_value_idx];

            co_yield std::make_unique<MoveSetPatient>(patient_index, patient_value.room_id, patient_value.operating_theater_id, patient_value.admission_day);
        }

        patient_index = (patient_index + 1) % problem.patients.size();
    }

    co_return;
}

Iterable<MoveSetPatient> MoveGeneratorSetPatient::random_moves_definitive(Solution& s) {
    for (int _ = 0; _ < problem.patients.size(); _++) {
        const Patient& patient = problem.patients[patient_index];

        for (int patient_value_idx : problem.patient_allowed_value_idxs_list[patient_index]) {
            const auto& patient_value = problem.patient_values[patient_value_idx];

            co_yield MoveSetPatient(patient_index, patient_value.room_id, patient_value.operating_theater_id, patient_value.admission_day);
        }

        patient_index = (patient_index + 1) % problem.patients.size();
    }

    co_return;
}

std::unique_ptr<Move> MoveGeneratorSetPatient::next(Solution& s) {
    int patient_id = s.distr_patient(gen);
    int patient_value_idx = problem.patient_allowed_value_idxs_list[patient_id][random_int(gen, 0, problem.patient_allowed_value_idxs_list[patient_id].size() - 1)];
    const auto& patient_value = problem.patient_values[patient_value_idx];

    return std::make_unique<MoveSetPatient>(patient_id, patient_value.room_id, patient_value.operating_theater_id, patient_value.admission_day);
}

Iterable<std::unique_ptr<Move>> MoveGeneratorSetNurse::random_moves(Solution& s) {
    for (int _ = 0; _ < problem.days; _++) {
        for (int shift = 0; shift < problem.num_shift_types; shift++) {
            auto& nurse_ids = problem.day_shift_nurse_ids[day_index][shift];

            for (int room_id = 0; room_id < problem.rooms.size(); room_id++) {
                int idx = room_id * problem.days * problem.num_shift_types + day_index * problem.num_shift_types + shift;
                int nurse_id_prev = s.sd.room_day_shift_nurse[idx];

                for (int nurse_id : nurse_ids) {
                    if (nurse_id == nurse_id_prev) {
                        continue;
                    }

                    auto move = std::make_unique<MoveSetNurse>(nurse_id, room_id, day_index, shift);
                    co_yield std::move(move);
                }
            }
        }
        day_index = (day_index + 1) % problem.days;
    }

    co_return;
}

Iterable<MoveSetNurse> MoveGeneratorSetNurse::random_moves_definitive(Solution& s) {
    for (int _ = 0; _ < problem.days; _++) {
        for (int shift = 0; shift < problem.num_shift_types; shift++) {
            auto& nurse_ids = problem.day_shift_nurse_ids[day_index][shift];

            for (int room_id = 0; room_id < problem.rooms.size(); room_id++) {
                int idx = room_id * problem.days * problem.num_shift_types + day_index * problem.num_shift_types + shift;
                int nurse_id_prev = s.sd.room_day_shift_nurse[idx];

                for (int nurse_id : nurse_ids) {
                    if (nurse_id == nurse_id_prev) {
                        continue;
                    }

                    co_yield MoveSetNurse(nurse_id, room_id, day_index, shift);
                }
            }
        }
        day_index = (day_index + 1) % problem.days;
    }

    co_return;
}

std::unique_ptr<Move> MoveGeneratorSetNurse::next(Solution& s) {
    int room_id = s.distr_room(gen);
    int day = s.distr_day(gen);
    int shift = s.distr_shift(gen);

    auto& nurse_ids = problem.day_shift_nurse_ids[day][shift];
    int nurse_id = nurse_ids[random_int(gen, 0, nurse_ids.size() - 1)];

    int idx = room_id * problem.days * problem.num_shift_types + day * problem.num_shift_types + shift;
    int nurse_id_prev = s.sd.room_day_shift_nurse[idx];
    if (nurse_id == nurse_id_prev) {
        return nullptr;
    }

    return std::make_unique<MoveSetNurse>(nurse_id, room_id, day, shift);
}

Iterable<std::unique_ptr<Move>> MoveGeneratorRemoveOptionalPatient::random_moves(Solution& s) {
    for (int _ = 0; _ < problem.optional_patient_ids.size(); _++) {
        int idx = problem.optional_patient_ids[patient_index];
        const Patient& patient = problem.patients[idx];

        if (s.sd.patient_room_ids[idx] != -1) {
            auto move = std::make_unique<MoveSetPatient>(idx, -1, -1, -1);
            co_yield std::move(move);
        }

        patient_index = (patient_index + 1) % problem.optional_patient_ids.size();
    }

    co_return;
}

Iterable<MoveSetPatient> MoveGeneratorRemoveOptionalPatient::random_moves_definitive(Solution& s) {
    for (int _ = 0; _ < problem.optional_patient_ids.size(); _++) {
        int idx = problem.optional_patient_ids[patient_index];
        const Patient& patient = problem.patients[idx];

        if (s.sd.patient_room_ids[idx] != -1) {
            //auto move = std::make_unique<MoveSetPatient>(idx, -1, -1, -1);
            //co_yield std::move(move);
            co_yield MoveSetPatient(idx, -1, -1, -1);
        }

        patient_index = (patient_index + 1) % problem.optional_patient_ids.size();
    }

    co_return;
}

std::unique_ptr<Move> MoveGeneratorRemoveOptionalPatient::next(Solution& s) {
    int patient_id = problem.optional_patient_ids[s.distr_optional_patient(gen)];
    if (s.sd.patient_room_ids[patient_id] == -1) {
        return nullptr;
    }
    
    return std::make_unique<MoveSetPatient>(patient_id, -1, -1, -1);
}

Iterable<std::unique_ptr<Move>> MoveGeneratorSwapPatients::random_moves(Solution& s) {
    for (int _ = 0; _ < problem.patients.size(); _++) {
        if (s.sd.patient_room_ids[patient_index] == -1) {
            patient_index = (patient_index + 1) % problem.patients.size();
            continue;
        }

        for (int j : problem.patient_swapable_ids[patient_index]) {
            if (s.sd.patient_room_ids[j] == -1) {
                continue;
            }

            int new_room_id1 = s.sd.patient_room_ids[j];
            int new_operating_theater_id1 = s.sd.patient_operating_theaters[j];
            int new_admission_day1 = s.sd.patient_admission_days[j];
            int value_idx1 = new_room_id1 * problem.days * problem.operating_theaters.size() + new_admission_day1 * problem.operating_theaters.size() + new_operating_theater_id1;
            if (problem.patient_allowed_value_idxs_set[patient_index].find(value_idx1) == problem.patient_allowed_value_idxs_set[patient_index].end()) {
                continue;
            }

            int new_room_id2 = s.sd.patient_room_ids[patient_index];
            int new_operating_theater_id2 = s.sd.patient_operating_theaters[patient_index];
            int new_admission_day2 = s.sd.patient_admission_days[patient_index];
            int value_idx2 = new_room_id2 * problem.days * problem.operating_theaters.size() + new_admission_day2 * problem.operating_theaters.size() + new_operating_theater_id2;
            if (problem.patient_allowed_value_idxs_set[j].find(value_idx2) == problem.patient_allowed_value_idxs_set[j].end()) {
                continue;
            }

            std::vector<std::unique_ptr<Move>> moves;
            moves.push_back(std::make_unique<MoveSetPatient>(patient_index, new_room_id1, new_operating_theater_id1, new_admission_day1));
            moves.push_back(std::make_unique<MoveSetPatient>(j, new_room_id2, new_operating_theater_id2, new_admission_day2));

            co_yield std::make_unique<MoveChain>(std::move(moves));
        }
        patient_index = (patient_index + 1) % problem.patients.size();
    }

    co_return;
}

Iterable<MoveChain> MoveGeneratorSwapPatients::random_moves_definitive(Solution& s) {
    for (int _ = 0; _ < problem.patients.size(); _++) {
        if (s.sd.patient_room_ids[patient_index] == -1) {
            patient_index = (patient_index + 1) % problem.patients.size();
            continue;
        }

        for (int j : problem.patient_swapable_ids[patient_index]) {
            if (s.sd.patient_room_ids[j] == -1) {
                continue;
            }

            int new_room_id1 = s.sd.patient_room_ids[j];
            int new_operating_theater_id1 = s.sd.patient_operating_theaters[j];
            int new_admission_day1 = s.sd.patient_admission_days[j];
            int value_idx1 = new_room_id1 * problem.days * problem.operating_theaters.size() + new_admission_day1 * problem.operating_theaters.size() + new_operating_theater_id1;
            if (problem.patient_allowed_value_idxs_set[patient_index].find(value_idx1) == problem.patient_allowed_value_idxs_set[patient_index].end()) {
                continue;
            }

            int new_room_id2 = s.sd.patient_room_ids[patient_index];
            int new_operating_theater_id2 = s.sd.patient_operating_theaters[patient_index];
            int new_admission_day2 = s.sd.patient_admission_days[patient_index];
            int value_idx2 = new_room_id2 * problem.days * problem.operating_theaters.size() + new_admission_day2 * problem.operating_theaters.size() + new_operating_theater_id2;
            if (problem.patient_allowed_value_idxs_set[j].find(value_idx2) == problem.patient_allowed_value_idxs_set[j].end()) {
                continue;
            }

            std::vector<std::unique_ptr<Move>> moves;
            moves.push_back(std::make_unique<MoveSetPatient>(patient_index, new_room_id1, new_operating_theater_id1, new_admission_day1));
            moves.push_back(std::make_unique<MoveSetPatient>(j, new_room_id2, new_operating_theater_id2, new_admission_day2));

            co_yield MoveChain(std::move(moves));
        }
        patient_index = (patient_index + 1) % problem.patients.size();
    }

    co_return;
}

std::unique_ptr<Move> MoveGeneratorSwapPatients::next(Solution& s) {
    int patient1_id = s.distr_patient(gen);
    if (s.sd.patient_room_ids[patient1_id] == -1) {
        return nullptr;
    }
    
    int patient2_id = s.distr_patient(gen);
    if (patient1_id == patient2_id || s.sd.patient_room_ids[patient2_id] == -1) {
        return nullptr;
    }

    int new_room_id1 = s.sd.patient_room_ids[patient2_id];
    int new_operating_theater_id1 = s.sd.patient_operating_theaters[patient2_id];
    int new_admission_day1 = s.sd.patient_admission_days[patient2_id];
    int value_idx1 = new_room_id1 * problem.days * problem.operating_theaters.size() + new_admission_day1 * problem.operating_theaters.size() + new_operating_theater_id1;
    if (problem.patient_allowed_value_idxs_set[patient1_id].find(value_idx1) == problem.patient_allowed_value_idxs_set[patient1_id].end()) {
        return nullptr;
    }

    int new_room_id2 = s.sd.patient_room_ids[patient1_id];
    int new_operating_theater_id2 = s.sd.patient_operating_theaters[patient1_id];
    int new_admission_day2 = s.sd.patient_admission_days[patient1_id];
    int value_idx2 = new_room_id2 * problem.days * problem.operating_theaters.size() + new_admission_day2 * problem.operating_theaters.size() + new_operating_theater_id2;
    if (problem.patient_allowed_value_idxs_set[patient2_id].find(value_idx2) == problem.patient_allowed_value_idxs_set[patient2_id].end()) {
        return nullptr;
    }

    std::vector<std::unique_ptr<Move>> moves;
    moves.push_back(std::make_unique<MoveSetPatient>(patient1_id, new_room_id1, new_operating_theater_id1, new_admission_day1));
    moves.push_back(std::make_unique<MoveSetPatient>(patient2_id, new_room_id2, new_operating_theater_id2, new_admission_day2));

    return std::make_unique<MoveChain>(std::move(moves));
}

Iterable<std::unique_ptr<Move>> MoveGeneratorKickPatient::random_moves(Solution& s) {
    for (int _ = 0; _ < problem.patients.size(); _++) {
        if (s.sd.patient_room_ids[patient_index] == -1) {
            patient_index = (patient_index + 1) % problem.patients.size();
            continue;
        }

        for (int j : problem.patient_kickable_ids[patient_index]) {
            if (s.sd.patient_room_ids[j] == -1) {
                continue;
            }

            int new_room_id1 = s.sd.patient_room_ids[j];
            int new_operating_theater_id1 = s.sd.patient_operating_theaters[j];
            int new_admission_day1 = s.sd.patient_admission_days[j];
            int value_idx1 = new_room_id1 * problem.days * problem.operating_theaters.size() + new_admission_day1 * problem.operating_theaters.size() + new_operating_theater_id1;
            if (problem.patient_allowed_value_idxs_set[patient_index].find(value_idx1) == problem.patient_allowed_value_idxs_set[patient_index].end()) {
                continue;
            }

            int value_idx2 = problem.patient_allowed_value_idxs_list[j][random_int(gen, 0, problem.patient_allowed_value_idxs_list[j].size() - 1)];
            const auto& patient_value2 = problem.patient_values[value_idx2];

            std::vector<std::unique_ptr<Move>> moves;
            moves.push_back(std::make_unique<MoveSetPatient>(patient_index, new_room_id1, new_operating_theater_id1, new_admission_day1));
            moves.push_back(std::make_unique<MoveSetPatient>(j, patient_value2.room_id, patient_value2.operating_theater_id, patient_value2.admission_day));

            co_yield std::make_unique<MoveChain>(std::move(moves));
        }

        patient_index = (patient_index + 1) % problem.patients.size();
    }

    co_return;
}

Iterable<MoveChain> MoveGeneratorKickPatient::random_moves_definitive(Solution& s) {
    for (int _ = 0; _ < problem.patients.size(); _++) {
        if (s.sd.patient_room_ids[patient_index] == -1) {
            patient_index = (patient_index + 1) % problem.patients.size();
            continue;
        }

        for (int j : problem.patient_kickable_ids[patient_index]) {
            if (s.sd.patient_room_ids[j] == -1) {
                continue;
            }

            int new_room_id1 = s.sd.patient_room_ids[j];
            int new_operating_theater_id1 = s.sd.patient_operating_theaters[j];
            int new_admission_day1 = s.sd.patient_admission_days[j];
            int value_idx1 = new_room_id1 * problem.days * problem.operating_theaters.size() + new_admission_day1 * problem.operating_theaters.size() + new_operating_theater_id1;
            if (problem.patient_allowed_value_idxs_set[patient_index].find(value_idx1) == problem.patient_allowed_value_idxs_set[patient_index].end()) {
                continue;
            }

            int value_idx2 = problem.patient_allowed_value_idxs_list[j][random_int(gen, 0, problem.patient_allowed_value_idxs_list[j].size() - 1)];
            const auto& patient_value2 = problem.patient_values[value_idx2];

            std::vector<std::unique_ptr<Move>> moves;
            moves.push_back(std::make_unique<MoveSetPatient>(patient_index, new_room_id1, new_operating_theater_id1, new_admission_day1));
            moves.push_back(std::make_unique<MoveSetPatient>(j, patient_value2.room_id, patient_value2.operating_theater_id, patient_value2.admission_day));

            co_yield MoveChain(std::move(moves));
        }

        patient_index = (patient_index + 1) % problem.patients.size();
    }

    co_return;
}

std::unique_ptr<Move> MoveGeneratorKickPatient::next(Solution& s) {
    int patient1_id = s.distr_patient(gen);
    if (s.sd.patient_room_ids[patient1_id] == -1) {
        return nullptr;
    }
    
    int patient2_id = s.distr_patient(gen);
    if (patient1_id == patient2_id || s.sd.patient_room_ids[patient2_id] == -1) {
        return nullptr;
    }

    int new_room_id1 = s.sd.patient_room_ids[patient2_id];
    int new_operating_theater_id1 = s.sd.patient_operating_theaters[patient2_id];
    int new_admission_day1 = s.sd.patient_admission_days[patient2_id];
    int value_idx1 = new_room_id1 * problem.days * problem.operating_theaters.size() + new_admission_day1 * problem.operating_theaters.size() + new_operating_theater_id1;
    if (problem.patient_allowed_value_idxs_set[patient1_id].find(value_idx1) == problem.patient_allowed_value_idxs_set[patient1_id].end()) {
        return nullptr;
    }

    int value_idx2 = problem.patient_allowed_value_idxs_list[patient2_id][random_int(gen, 0, problem.patient_allowed_value_idxs_list[patient2_id].size() - 1)];
    const auto& patient_value2 = problem.patient_values[value_idx2];

    std::vector<std::unique_ptr<Move>> moves;
    moves.push_back(std::make_unique<MoveSetPatient>(patient1_id, new_room_id1, new_operating_theater_id1, new_admission_day1));
    moves.push_back(std::make_unique<MoveSetPatient>(patient2_id, patient_value2.room_id, patient_value2.operating_theater_id, patient_value2.admission_day));

    return std::make_unique<MoveChain>(std::move(moves));
}

Iterable<std::unique_ptr<Move>> MoveGeneratorSwapNursesAll::random_moves(Solution& s) {
    for (int _ = 0; _ < problem.days; _++) {
        for (int shift = 0; shift < problem.num_shift_types; shift++) {
            auto& nurse_ids = problem.day_shift_nurse_ids[day_index][shift];

            for (int i = 0; i < nurse_ids.size(); i++) {
                for (int j = i + 1; j < nurse_ids.size(); j++) {
                    int nurse_id1 = nurse_ids[i];
                    int nurse_id2 = nurse_ids[j];

                    auto& nurse_room_ids1 = s.sd.nurse_day_shift_rooms[nurse_id1][day_index][shift];
                    auto& nurse_room_ids2 = s.sd.nurse_day_shift_rooms[nurse_id2][day_index][shift];

                    std::vector<std::unique_ptr<Move>> moves;
                    for (int room_id : nurse_room_ids1) {
                        moves.push_back(std::make_unique<MoveSetNurse>(nurse_id2, room_id, day_index, shift));
                    }
                    for (int room_id : nurse_room_ids2) {
                        moves.push_back(std::make_unique<MoveSetNurse>(nurse_id1, room_id, day_index, shift));
                    }

                    if (moves.size() < 2) {
                        continue;
                    }

                    co_yield std::make_unique<MoveChain>(std::move(moves));
                }
            }
        }

        day_index = (day_index + 1) % problem.days;
    }

    co_return;
}

Iterable<MoveChain> MoveGeneratorSwapNursesAll::random_moves_definitive(Solution& s) {
    for (int _ = 0; _ < problem.days; _++) {
        for (int shift = 0; shift < problem.num_shift_types; shift++) {
            auto& nurse_ids = problem.day_shift_nurse_ids[day_index][shift];

            for (int i = 0; i < nurse_ids.size(); i++) {
                for (int j = i + 1; j < nurse_ids.size(); j++) {
                    int nurse_id1 = nurse_ids[i];
                    int nurse_id2 = nurse_ids[j];

                    auto& nurse_room_ids1 = s.sd.nurse_day_shift_rooms[nurse_id1][day_index][shift];
                    auto& nurse_room_ids2 = s.sd.nurse_day_shift_rooms[nurse_id2][day_index][shift];

                    std::vector<std::unique_ptr<Move>> moves;
                    for (int room_id : nurse_room_ids1) {
                        moves.push_back(std::make_unique<MoveSetNurse>(nurse_id2, room_id, day_index, shift));
                    }
                    for (int room_id : nurse_room_ids2) {
                        moves.push_back(std::make_unique<MoveSetNurse>(nurse_id1, room_id, day_index, shift));
                    }

                    if (moves.size() <= 2) {
                        continue;
                    }

                    co_yield MoveChain(std::move(moves));
                }
            }
        }

        day_index = (day_index + 1) % problem.days;
    }

    co_return;
}

std::unique_ptr<Move> MoveGeneratorSwapNursesAll::next(Solution& s) {
    int day = s.distr_day(gen);
    int shift = s.distr_shift(gen);
    auto& nurse_ids = problem.day_shift_nurse_ids[day][shift];

    if (nurse_ids.size() < 2) {
        return nullptr;
    }

    int nurse_id1 = nurse_ids[random_int(gen, 0, nurse_ids.size() - 1)];
    int nurse_id2_idx = random_int(gen, 0, nurse_ids.size() - 1);
    if (nurse_id1 == nurse_ids[nurse_id2_idx]) {
        nurse_id2_idx = (nurse_id2_idx + 1) % nurse_ids.size();
    }
    int nurse_id2 = nurse_ids[nurse_id2_idx];

    auto& nurse_room_ids1 = s.sd.nurse_day_shift_rooms[nurse_id1][day][shift];
    auto& nurse_room_ids2 = s.sd.nurse_day_shift_rooms[nurse_id2][day][shift];

    std::vector<std::unique_ptr<Move>> moves;
    for (int room_id : nurse_room_ids1) {
        moves.push_back(std::make_unique<MoveSetNurse>(nurse_id2, room_id, day, shift));
    }
    for (int room_id : nurse_room_ids2) {
        moves.push_back(std::make_unique<MoveSetNurse>(nurse_id1, room_id, day, shift));
    }

    if (moves.size() < 2) {
        return nullptr;
    }

    return std::make_unique<MoveChain>(std::move(moves));
}

Iterable<std::unique_ptr<Move>> MoveGeneratorSwapNursesSingle::random_moves(Solution& s) {
    for (int _ = 0; _ < problem.days; _++) {
        for (int shift = 0; shift < problem.num_shift_types; shift++) {
            auto& nurse_ids = problem.day_shift_nurse_ids[day_index][shift];

            for (int i = 0; i < nurse_ids.size(); i++) {
                for (int j = i + 1; j < nurse_ids.size(); j++) {
                    int nurse_id1 = nurse_ids[i];
                    int nurse_id2 = nurse_ids[j];

                    auto& nurse_room_ids1 = s.sd.nurse_day_shift_rooms[nurse_id1][day_index][shift];
                    auto& nurse_room_ids2 = s.sd.nurse_day_shift_rooms[nurse_id2][day_index][shift];


                    for (int room_idx1 = nurse_room_ids1.size() - 1; room_idx1 >= 0; room_idx1--) {
                        int room_id1 = nurse_room_ids1[room_idx1];

                        for (int room_idx2 = nurse_room_ids2.size() - 1; room_idx2 >= 0; room_idx2--) {
                            int room_id2 = nurse_room_ids2[room_idx2];
                            
                            std::vector<std::unique_ptr<Move>> moves;

                            moves.push_back(std::make_unique<MoveSetNurse>(nurse_id1, room_id2, day_index, shift));
                            moves.push_back(std::make_unique<MoveSetNurse>(nurse_id2, room_id1, day_index, shift));

                            co_yield std::make_unique<MoveChain>(std::move(moves));
                        }
                    }
                }
            }
        }

        day_index = (day_index + 1) % problem.days;
    }
    
    co_return;
}

Iterable<MoveChain> MoveGeneratorSwapNursesSingle::random_moves_definitive(Solution& s) {
    for (int _ = 0; _ < problem.days; _++) {
        for (int shift = 0; shift < problem.num_shift_types; shift++) {
            auto& nurse_ids = problem.day_shift_nurse_ids[day_index][shift];

            for (int i = 0; i < nurse_ids.size(); i++) {
                for (int j = i + 1; j < nurse_ids.size(); j++) {
                    int nurse_id1 = nurse_ids[i];
                    int nurse_id2 = nurse_ids[j];

                    auto& nurse_room_ids1 = s.sd.nurse_day_shift_rooms[nurse_id1][day_index][shift];
                    auto& nurse_room_ids2 = s.sd.nurse_day_shift_rooms[nurse_id2][day_index][shift];


                    for (int room_idx1 = nurse_room_ids1.size() - 1; room_idx1 >= 0; room_idx1--) {
                        int room_id1 = nurse_room_ids1[room_idx1];

                        for (int room_idx2 = nurse_room_ids2.size() - 1; room_idx2 >= 0; room_idx2--) {
                            int room_id2 = nurse_room_ids2[room_idx2];
                            
                            std::vector<std::unique_ptr<Move>> moves;

                            moves.push_back(std::make_unique<MoveSetNurse>(nurse_id1, room_id2, day_index, shift));
                            moves.push_back(std::make_unique<MoveSetNurse>(nurse_id2, room_id1, day_index, shift));

                            co_yield MoveChain(std::move(moves));
                        }
                    }
                }
            }
        }

        day_index = (day_index + 1) % problem.days;
    }
    
    co_return;
}

std::unique_ptr<Move> MoveGeneratorSwapNursesSingle::next(Solution& s) {
    int day = s.distr_day(gen);
    int shift = s.distr_shift(gen);
    auto& nurse_ids = problem.day_shift_nurse_ids[day][shift];

    if (nurse_ids.size() < 2) {
        return nullptr;
    }

    int nurse_id1 = nurse_ids[random_int(gen, 0, nurse_ids.size() - 1)];
    int nurse_id2_idx = random_int(gen, 0, nurse_ids.size() - 1);
    if (nurse_id1 == nurse_ids[nurse_id2_idx]) {
        nurse_id2_idx = (nurse_id2_idx + 1) % nurse_ids.size();
    }
    int nurse_id2 = nurse_ids[nurse_id2_idx];

    auto& nurse_room_ids1 = s.sd.nurse_day_shift_rooms[nurse_id1][day][shift];
    auto& nurse_room_ids2 = s.sd.nurse_day_shift_rooms[nurse_id2][day][shift];
    if (nurse_room_ids1.size() == 0 || nurse_room_ids2.size() == 0) {
        return nullptr;
    }

    int room_id1 = nurse_room_ids1[random_int(gen, 0, nurse_room_ids1.size() - 1)];
    int room_id2 = nurse_room_ids2[random_int(gen, 0, nurse_room_ids2.size() - 1)];

    std::vector<std::unique_ptr<Move>> moves;
    moves.push_back(std::make_unique<MoveSetNurse>(nurse_id1, room_id2, day, shift));
    moves.push_back(std::make_unique<MoveSetNurse>(nurse_id2, room_id1, day, shift));

    return std::make_unique<MoveChain>(std::move(moves));
}

Iterable<std::unique_ptr<Move>> MoveGeneratorKickPatientOut::random_moves(Solution& s) {
    for (int _ = 0; _ < problem.patients.size(); _++) {
        if (s.sd.patient_room_ids[patient_index] == -1) {
            patient_index = (patient_index + 1) % problem.patients.size();
            continue;
        }

        for (int j : problem.patient_kickableout_ids[patient_index]) {
            if (s.sd.patient_room_ids[j] == -1) {
                continue;
            }

            int new_room_id1 = s.sd.patient_room_ids[j];
            int new_operating_theater_id1 = s.sd.patient_operating_theaters[j];
            int new_admission_day1 = s.sd.patient_admission_days[j];
            int value_idx1 = new_room_id1 * problem.days * problem.operating_theaters.size() + new_admission_day1 * problem.operating_theaters.size() + new_operating_theater_id1;
            if (problem.patient_allowed_value_idxs_set[patient_index].find(value_idx1) == problem.patient_allowed_value_idxs_set[patient_index].end()) {
                continue;
            }

            std::vector<std::unique_ptr<Move>> moves;
            moves.push_back(std::make_unique<MoveSetPatient>(patient_index, new_room_id1, new_operating_theater_id1, new_admission_day1));
            moves.push_back(std::make_unique<MoveSetPatient>(j, -1, -1, -1));

            co_yield std::make_unique<MoveChain>(std::move(moves));
        }
        patient_index = (patient_index + 1) % problem.patients.size();
    }

    co_return;
}

Iterable<MoveChain> MoveGeneratorKickPatientOut::random_moves_definitive(Solution& s) {
    for (int _ = 0; _ < problem.patients.size(); _++) {
        if (s.sd.patient_room_ids[patient_index] == -1) {
            patient_index = (patient_index + 1) % problem.patients.size();
            continue;
        }

        for (int j : problem.patient_kickableout_ids[patient_index]) {
            if (s.sd.patient_room_ids[j] == -1) {
                continue;
            }

            int new_room_id1 = s.sd.patient_room_ids[j];
            int new_operating_theater_id1 = s.sd.patient_operating_theaters[j];
            int new_admission_day1 = s.sd.patient_admission_days[j];
            int value_idx1 = new_room_id1 * problem.days * problem.operating_theaters.size() + new_admission_day1 * problem.operating_theaters.size() + new_operating_theater_id1;
            if (problem.patient_allowed_value_idxs_set[patient_index].find(value_idx1) == problem.patient_allowed_value_idxs_set[patient_index].end()) {
                continue;
            }

            std::vector<std::unique_ptr<Move>> moves;
            moves.push_back(std::make_unique<MoveSetPatient>(patient_index, new_room_id1, new_operating_theater_id1, new_admission_day1));
            moves.push_back(std::make_unique<MoveSetPatient>(j, -1, -1, -1));

            co_yield MoveChain(std::move(moves));
        }
        patient_index = (patient_index + 1) % problem.patients.size();
    }

    co_return;
}

std::unique_ptr<Move> MoveGeneratorKickPatientOut::next(Solution& s) {
    int patient1_id = s.distr_patient(gen);
    if (s.sd.patient_room_ids[patient1_id] == -1) {
        return nullptr;
    }
    
    int patient2_id = problem.optional_patient_ids[s.distr_optional_patient(gen)];
    if (patient1_id == patient2_id || s.sd.patient_room_ids[patient2_id] == -1) {
        return nullptr;
    }

    int new_room_id1 = s.sd.patient_room_ids[patient2_id];
    int new_operating_theater_id1 = s.sd.patient_operating_theaters[patient2_id];
    int new_admission_day1 = s.sd.patient_admission_days[patient2_id];
    int value_idx1 = new_room_id1 * problem.days * problem.operating_theaters.size() + new_admission_day1 * problem.operating_theaters.size() + new_operating_theater_id1;
    if (problem.patient_allowed_value_idxs_set[patient1_id].find(value_idx1) == problem.patient_allowed_value_idxs_set[patient1_id].end()) {
        return nullptr;
    }

    std::vector<std::unique_ptr<Move>> moves;
    moves.push_back(std::make_unique<MoveSetPatient>(patient1_id, new_room_id1, new_operating_theater_id1, new_admission_day1));
    moves.push_back(std::make_unique<MoveSetPatient>(patient2_id, -1, -1, -1));

    return std::make_unique<MoveChain>(std::move(moves));
}
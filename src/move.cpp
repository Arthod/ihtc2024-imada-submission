
#include "move.h"
#include "solution_data.h"

void MoveSetPatient::make(SolutionData& sd) {
    old_room_id = sd.patient_room_ids[patient_id];
    old_operating_theater_id = sd.patient_operating_theaters[patient_id];
    old_admission_day = sd.patient_admission_days[patient_id];

    sd.patient_room_ids[patient_id] = new_room_id;
    sd.patient_operating_theaters[patient_id] = new_operating_theater_id;
    sd.patient_admission_days[patient_id] = new_admission_day;

    if (old_room_id == -1) {
        sd.patients_scheduled.insert(patient_id);
    } else if (new_room_id == -1) {
        sd.patients_scheduled.erase(patient_id);
    }
}

void MoveSetPatient::undo(SolutionData& sd) {
    sd.patient_room_ids[patient_id] = old_room_id;
    sd.patient_operating_theaters[patient_id] = old_operating_theater_id;
    sd.patient_admission_days[patient_id] = old_admission_day;

    if (new_room_id == -1) {
        sd.patients_scheduled.insert(patient_id);
    } else if (old_room_id == -1) {
        sd.patients_scheduled.erase(patient_id);
    }
}

std::unique_ptr<Move> MoveSetPatient::get_undo_move() {
    return std::make_unique<MoveSetPatient>(patient_id, old_room_id, old_operating_theater_id, old_admission_day);
}

int MoveSetPatient::get_incr(EvaluationElement& eval_elem, SolutionData& sd, bool is_commit) {
    return eval_elem.get_incr(sd, *this, is_commit);
}

void MoveSetNurse::make(SolutionData& sd) {
    int idx = room_id * sd.problem.days * sd.problem.num_shift_types + day * sd.problem.num_shift_types + shift;
    old_nurse_id = sd.room_day_shift_nurse[idx];
    if (old_nurse_id == nurse_id) {
        return;
    }

    auto& old_nurse_rooms = sd.nurse_day_shift_rooms[old_nurse_id][day][shift];
    old_nurse_rooms.erase(std::remove(old_nurse_rooms.begin(), old_nurse_rooms.end(), room_id), old_nurse_rooms.end());
    sd.nurse_day_shift_rooms[nurse_id][day][shift].push_back(room_id);

    sd.room_day_shift_nurse[idx] = nurse_id;
}

void MoveSetNurse::undo(SolutionData& sd) {
    if (old_nurse_id == nurse_id) {
        return;
    }

    auto& new_nurse_rooms = sd.nurse_day_shift_rooms[nurse_id][day][shift];
    new_nurse_rooms.erase(std::remove(new_nurse_rooms.begin(), new_nurse_rooms.end(), room_id), new_nurse_rooms.end());
    sd.nurse_day_shift_rooms[old_nurse_id][day][shift].push_back(room_id);

    int idx = room_id * sd.problem.days * sd.problem.num_shift_types + day * sd.problem.num_shift_types + shift;
    sd.room_day_shift_nurse[idx] = old_nurse_id;
}

std::unique_ptr<Move> MoveSetNurse::get_undo_move() {
    return std::make_unique<MoveSetNurse>(old_nurse_id, room_id, day, shift);
}

int MoveSetNurse::get_incr(EvaluationElement& eval_elem, SolutionData& sd, bool is_commit) {
    return eval_elem.get_incr(sd, *this, is_commit);
}

void MoveChain::make(SolutionData& sd) {
    for (auto& m : moves) {
        m->make(sd);
    }
}

void MoveChain::undo(SolutionData& sd) {
    for (auto it = moves.rbegin(); it != moves.rend(); it++) {
        (*it)->undo(sd);
    }
}

std::unique_ptr<Move> MoveChain::get_undo_move() {
    throw std::runtime_error("unsupported");
}

int MoveChain::get_incr(EvaluationElement& eval_elem, SolutionData& sd, bool is_commit) {
    int incr = 0;
        
    for (int i = 0; i < moves.size() - 1; i++) {
        auto& m = moves[i];
        incr += m->get_incr(eval_elem, sd, true);
        m->make(sd);
    }
    auto& move_last = moves[moves.size() - 1];
    incr += move_last->get_incr(eval_elem, sd, false);

    for (int i = moves.size() - 2; i >= 0; i--) {
        auto& m = moves[i];
        auto undo_move = m->get_undo_move();
        undo_move->get_incr(eval_elem, sd, true);
        m->undo(sd);
    }

    return incr;
}


#include <iostream>
#include <string>
#include "problem.h"
#include "solution.h"
#include "solution_data.h"
#include "move.h"
#include "evaluation_element.h"
#include "evaluation.h"

#include "config.h"

Solution::Solution(const Problem& problem, SolutionData& sd, Evaluation& evaluation) : problem(problem), sd(sd), evaluation(evaluation) {
}

Solution::Solution(const Problem& problem, SolutionData& sd) : problem(problem), sd(sd), evaluation(Evaluation(std::vector<int>(4, INT_MAX))) {
}


Evaluation Solution::get_objective(bool is_debug) {
    std::vector<int> values(eval_elems.size(), 0);
    for (int i = 0; i < eval_elems.size(); i++) {
        values[i] = eval_elems[i]->get_objective(sd, is_debug);
    }
    return Evaluation(values);
}

Evaluation Solution::get_incr(SolutionData& sd, Move& move, bool is_commit) {
    if (sd.is_lazy_copy) {
        evaluation = get_objective(false);
        sd.is_lazy_copy = false;
    }

    std::vector<int> values(eval_elems.size(), 0);
    if (auto *move_ = dynamic_cast<MoveChain*>(&move)) {
#if DEBUG >= 2
        if (move_->moves.size() < 2) {
            std::cout << "Error: MoveChain must have at least 2 moves, but has " << move_->moves.size() << std::endl;
            exit(1);
        }
#endif
        for (int i = 0; i < move_->moves.size() - 1; i++) {
            auto& m = move_->moves[i];
            for (int j = 0; j < eval_elems.size(); j++) {
                values[j] += m->get_incr(*eval_elems[j], sd, true);
            }
            m->make(sd);
        }
        auto& move_last = move_->moves[move_->moves.size() - 1];
        for (int i = 0; i < eval_elems.size(); i++) {
            values[i] += move_last->get_incr(*eval_elems[i], sd, is_commit);
        }
        if (is_commit) {
            move_last->make(sd);
        }

        if (!is_commit) {
            for (int i = move_->moves.size() - 2; i >= 0; i--) {
                auto& m = move_->moves[i];
                auto undo_move = m->get_undo_move();
                for (int j = 0; j < eval_elems.size(); j++) {
                    undo_move->get_incr(*eval_elems[j], sd, true);
                }
                m->undo(sd);
            }
        }
            
    } else {
        for (int i = 0; i < eval_elems.size(); i++) {
            values[i] = move.get_incr(*eval_elems[i], sd, is_commit);
        }
        if (is_commit) {
            move.make(sd);
        }
    }

#if DEBUG >= 3
        // Assert that the increment is correct
        if (!is_commit) {
            move.make(sd);
            Evaluation evaluation_new = get_objective(true);
            Evaluation evaluation_est = evaluation + Evaluation(values);
            if (evaluation_new != evaluation_est) {
                std::cout << "DELTA INCR ERROR: Move type: " << typeid(move).name() << ", Is=" << evaluation_est.to_string() << " != Should=" << evaluation_new.to_string() << std::endl;
                for (int i = 0; i < eval_elems.size(); i++) {
                    if (values[i] != evaluation_new.values[i] - evaluation.values[i]) {
                        std::cout << "Error with: " << typeid(*eval_elems[i]).name() << ", Is=" << values[i] << " != Should=" << evaluation_new.values[i] - evaluation.values[i] << std::endl;
                    }
                }
                exit(1);
#if DEBUG_VERBOSE > 1
            } else {
                std::cout << "LVL3 DELTA INCR OK: Move type: " << typeid(move).name() << ", Is=" << evaluation_est.to_string() << " == Should=" << evaluation_new.to_string() << std::endl;
#endif
            }
            move.undo(sd);
        }
#endif

    return Evaluation(values);
}

void Solution::step(Move& move) {
    Evaluation incr = get_incr(sd, move, true);
    evaluation = evaluation + incr;

#if DEBUG >= 2
    Evaluation evaluation_real = get_objective(true);
    if (evaluation_real != evaluation) {
#if DEBUG_VERBOSE > 0
        std::cout << "DELTA MOVE ERROR: Move type: " << typeid(move).name() << ", Is=" << evaluation.to_string() << " != Should=" << evaluation_real.to_string() << std::endl;
#endif
        for (int i = 0; i < eval_elems.size(); i++) {
            if (evaluation.values[i] != evaluation_real.values[i]) {
                std::cout << "Error with: " << typeid(*eval_elems[i]).name() << ", Is=" << evaluation.values[i] << " != Should=" << evaluation_real.values[i] << std::endl;
            }
        }
        exit(1);
#if DEBUG_VERBOSE > 1
    } else {
        std::cout << "LVL2 DELTA MOVE OK: Move type: " << typeid(move).name() << ", Is=" << evaluation.to_string() << " == Should=" << evaluation_real.to_string() << std::endl;
#endif
    }
#endif

#if DEBUG >= 3
    // Assert that all patients are legal
    for (int i = 0; i < problem.patients.size(); i++) {
        const Patient& patient = problem.patients[i];

        int room_unscheduled = sd.patient_room_ids[i] == -1 ? 1 : 0;
        int operating_theater_unscheduled = sd.patient_operating_theaters[i] == -1 ? 1 : 0;
        int admission_day_unscheduled = sd.patient_admission_days[i] == -1 ? 1 : 0;

        // All must be unscheduled or all must be scheduled
        if (room_unscheduled != operating_theater_unscheduled || room_unscheduled != admission_day_unscheduled) {
            std::cout << "ERROR: Patient " << i << " is not legal" << std::endl;
            exit(1);
        }

        // Room can only be unscheduled if all are unscheduled
        if (room_unscheduled == 1 && operating_theater_unscheduled == 1 && admission_day_unscheduled == 1) {
            if (patient.mandatory) {
                std::cout << "ERROR: Mandatory patient is unscheduled" << std::endl;
                exit(1);
            }
        }

        // Is scheduled in eligible room, operating theater and admission day
        if (room_unscheduled == 0) {
            int value_idx = sd.patient_room_ids[i] * problem.days * problem.operating_theaters.size() + sd.patient_admission_days[i] * problem.operating_theaters.size() + sd.patient_operating_theaters[i];
            if (problem.patient_allowed_value_idxs_set[i].find(value_idx) == problem.patient_allowed_value_idxs_set[i].end()) {
                std::cout << "ERROR: Patient " << i << " is scheduled in ineligible value index" << std::endl;
                exit(1);
            }
        }
    }
#endif
}
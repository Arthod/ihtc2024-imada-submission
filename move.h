#ifndef MOVE_H
#define MOVE_H

#include <memory>

#include "solution_data.h"

class EvaluationElement;
class Move;


class Move {
public:
    virtual ~Move() = default;

    virtual void make(SolutionData& sd) = 0;
    virtual void undo(SolutionData& sd) = 0;
    virtual std::unique_ptr<Move> get_undo_move() = 0;
    virtual int get_incr(EvaluationElement& eval_elem, SolutionData& sd, bool is_commit) = 0;
};

class MoveSetPatient : public Move {
public:
    int patient_id;
    int new_room_id;
    int new_operating_theater_id;
    int new_admission_day;

    int old_room_id = -1;
    int old_operating_theater_id = -1;
    int old_admission_day = -1;

    MoveSetPatient(int patient_id, int new_room_id, int new_operating_theater_id, int new_admission_day)
        : patient_id(patient_id), new_room_id(new_room_id), new_operating_theater_id(new_operating_theater_id), new_admission_day(new_admission_day) {}
    void make(SolutionData& sd) override;
    void undo(SolutionData& sd) override;
    std::unique_ptr<Move> get_undo_move() override;
    int get_incr(EvaluationElement& eval_elem, SolutionData& sd, bool is_commit) override;
};

class MoveSetNurse : public Move {
public:
    int nurse_id;
    int room_id;
    int day;
    int shift;

    int old_nurse_id = -1;

    MoveSetNurse(int nurse_id, int room_id, int day, int shift) 
        : nurse_id(nurse_id), room_id(room_id), day(day), shift(shift) {}
    void make(SolutionData& sd) override;
    void undo(SolutionData& sd) override;
    std::unique_ptr<Move> get_undo_move() override;
    int get_incr(EvaluationElement& eval_elem, SolutionData& sd, bool is_commit) override;
};

class MoveChain : public Move {
public:
    std::vector<std::unique_ptr<Move>> moves;

    MoveChain(std::vector<std::unique_ptr<Move>> moves) : moves(std::move(moves)) {}
    void make(SolutionData& sd) override;
    void undo(SolutionData& sd) override;
    std::unique_ptr<Move> get_undo_move() override;
    int get_incr(EvaluationElement& eval_elem, SolutionData& sd, bool is_commit) override;
};


class EvaluationElement {
    public:
        virtual int get_objective(SolutionData& sd, bool is_debug) = 0;
        virtual int get_incr(SolutionData& sd, MoveSetPatient& move, bool is_commit) = 0;
        virtual int get_incr(SolutionData& sd, MoveSetNurse& move, bool is_commit) = 0;
    };

#endif
#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>
#include <memory>

#include "problem.h"
#include "solution_data.h"
#include "evaluation.h"
#include "evaluation_element.h"
#include "move.h"

class Solution {
public:
    const Problem& problem;
    SolutionData& sd;
    Evaluation evaluation;

    std::vector<std::unique_ptr<EvaluationElement>> eval_elems;
    
    int move_generator_index = 0;

    Solution(const Problem& problem, SolutionData& sd, Evaluation& evaluation);
    Solution(const Problem& problem, SolutionData& sd);

    Evaluation get_objective(bool is_debug);
    Evaluation get_incr(SolutionData& sd, Move& move, bool is_commit);
    void step(Move& move);

    std::uniform_int_distribution<int> distr_day = std::uniform_int_distribution<int>(0, problem.days - 1);
    std::uniform_int_distribution<int> distr_shift = std::uniform_int_distribution<int>(0, problem.num_shift_types - 1);
    std::uniform_int_distribution<int> distr_patient = std::uniform_int_distribution<int>(0, problem.patients.size() - 1);
    std::uniform_int_distribution<int> distr_optional_patient = std::uniform_int_distribution<int>(0, problem.optional_patient_ids.size() - 1);
    std::uniform_int_distribution<int> distr_room = std::uniform_int_distribution<int>(0, problem.rooms.size() - 1);
    std::uniform_int_distribution<int> distr_operating_theater = std::uniform_int_distribution<int>(0, problem.operating_theaters.size() - 1);
    std::uniform_int_distribution<int> distr_nurse = std::uniform_int_distribution<int>(0, problem.nurses.size() - 1);
};

#endif
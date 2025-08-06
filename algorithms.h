#include "solution.h"
#include "move_generator.h"
#include <random>
#include "move.h"
#include "evaluation_element.h"

void sa_plus(Solution& s, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, MoveGeneratorUnion& mg, std::mt19937& gen, int duration, std::vector<double> temperatures, std::vector<double> temperature_duration_fractions, int debug_level);
void fi_plus(Solution& s, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, MoveGenerator& mg, std::mt19937& gen, int duration, int debug_level);
void fi_randomsubset_plus(Solution& s, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, MoveGenerator& mg, std::mt19937& gen, int duration, int subset_size, int debug_level);
void ils(Solution& s, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, MoveGenerator& mg, PerturbatorUnion& pu, double ks, std::mt19937& gen, int duration, int debug_level, int temptol);
void ils2(Solution& s, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, MoveGeneratorUnion& mgu, PerturbatorUnion& pu, double ks, std::mt19937& gen, int duration, int debug_level, int temptol);

#ifndef PERTURBATOR_H
#define PERTURBATOR_H

#include <memory>
#include "solution.h"
#include "move.h"
#include "evaluation.h"
#include "move_generator.h"
#include "evaluation_element.h"

class Perturbator {
public:
    virtual ~Perturbator() = default;

    virtual int perturb(Solution& s, double ks) = 0;
    virtual std::string name() { return typeid(*this).name(); }
};

class PerturbatorUnion : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;

    std::vector<std::unique_ptr<Perturbator>> pus;
    std::vector<double> weights;
    std::vector<int> perturbator_indices;
    std::uniform_int_distribution<int> distr_perturbator_index;

    PerturbatorUnion(const Problem& problem, std::mt19937& gen, std::vector<std::pair<Perturbator*, double>>& pu_weights);
    int perturb(Solution& s, double ks) override;
    void compute_weight_indices();
};

class PerturbatorCloseOperatingTheaters : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    double ks_min;
    double ks_max;

    PerturbatorCloseOperatingTheaters(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
};

class PerturbatorCloseOperatingTheaterDays : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    double ks_min;
    double ks_max;

    PerturbatorCloseOperatingTheaterDays(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
};

class PerturbatorCloseRooms : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    double ks_min;
    double ks_max;

    PerturbatorCloseRooms(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
};

class PerturbatorRemoveOptionalPatients : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    double ks_min;
    double ks_max;

    PerturbatorRemoveOptionalPatients(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
};

class PerturbatorEnforceAdmissionDay : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    double ks_min;
    double ks_max;

    PerturbatorEnforceAdmissionDay(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
};

class PerturbatorFromGenerator : public Perturbator {
public:
    const Problem& problem;
    std::unique_ptr<MoveGenerator> mg;
    int size;
    double ks_min;
    double ks_max;

    PerturbatorFromGenerator(const Problem& problem, std::unique_ptr<MoveGenerator> mg, int size, double ks_min, double ks_max)
        : problem(problem), mg(std::move(mg)), size(size), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
    std::string name() override { return "PFG(" + std::string(typeid(*mg).name()) + ")"; };
};
    
class PerturbatorPermutatePatientRooms : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    std::vector<int> room_idxs;
    double ks_min;
    double ks_max;

    PerturbatorPermutatePatientRooms(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {
            for (int i = 0; i < problem.rooms.size(); i++) {
                room_idxs.push_back(i);
            }
        }
    int perturb(Solution& s, double ks) override;
};

class PerturbatorPermutatePatientOperatingTheaters : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    std::vector<int> operating_theater_idxs;
    double ks_min;
    double ks_max;

    PerturbatorPermutatePatientOperatingTheaters(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {
            for (int i = 0; i < problem.operating_theaters.size(); i++) {
                operating_theater_idxs.push_back(i);
            }
        }
    int perturb(Solution& s, double ks) override;
};

class PerturbatorChangeWeights : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    MoveGenerator& mg;
    EvalElemHardConstraints& eval_elem_hard_constraints;
    EvalElemSoftConstraints& eval_elem_soft_constraints;
    double ks_min;
    double ks_max;
    int last_index;

    PerturbatorChangeWeights(const Problem& problem, std::mt19937& gen, MoveGenerator& mg, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, double ks_min, double ks_max, int last_index)
        : problem(problem), gen(gen), mg(mg), eval_elem_hard_constraints(eval_elem_hard_constraints), eval_elem_soft_constraints(eval_elem_soft_constraints), ks_min(ks_min), ks_max(ks_max), last_index(last_index) {}
    int perturb(Solution& s, double ks) override;
};

class PerturbatorShuffleEquivalentNurses : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    double ks_min;
    double ks_max;

    PerturbatorShuffleEquivalentNurses(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
};

class PerturbatorPatientAdmissionDayOffset : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    int offset;
    double ks_min;
    double ks_max;

    PerturbatorPatientAdmissionDayOffset(const Problem& problem, std::mt19937& gen, int offset, double ks_min, double ks_max)
        : problem(problem), gen(gen), offset(offset), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
    std::string name() override { return "PatientAdmissionDayOffset(" + std::to_string(offset) + ")"; };
};

class PerturbatorPermutatePatientAdmissionDays : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    std::vector<int> admission_days;
    double ks_min;
    double ks_max;

    PerturbatorPermutatePatientAdmissionDays(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {
            for (int i = 0; i < problem.days; i++) {
                admission_days.push_back(i);
            }
        }
    int perturb(Solution& s, double ks) override;
};

class PerturbatorAssignUnscheduledPatientsBest : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    double ks_min;
    double ks_max;

    PerturbatorAssignUnscheduledPatientsBest(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
};

class PerturbatorSurgeonDayRemoveSingleOT : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    double ks_min;
    double ks_max;

    PerturbatorSurgeonDayRemoveSingleOT(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
};

class PerturbatorNursePatientShuffleLowCareCount : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    double ks_min;
    double ks_max;

    PerturbatorNursePatientShuffleLowCareCount(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
};

class PerturbatorNurseShuffleOverWorkload : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    double ks_min;
    double ks_max;

    PerturbatorNurseShuffleOverWorkload(const Problem& problem, std::mt19937& gen, double ks_min, double ks_max)
        : problem(problem), gen(gen), ks_min(ks_min), ks_max(ks_max) {}
    int perturb(Solution& s, double ks) override;
};

class PerturbatorSetPatientsMany : public Perturbator {
public:
    const Problem& problem;
    std::mt19937& gen;
    int moves_min;
    int moves_max;

    PerturbatorSetPatientsMany(const Problem& problem, std::mt19937& gen, int moves_min, int moves_max)
        : problem(problem), gen(gen), moves_min(moves_min), moves_max(moves_max) {}
    int perturb(Solution& s, double ks) override;
};

#endif
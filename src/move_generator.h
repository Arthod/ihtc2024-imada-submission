#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include <memory>
#include "solution.h"
#include "iterable.h"
#include "move.h"

#include "config.h"

class MoveGenerator {
public:
    virtual ~MoveGenerator() = default;

    virtual Iterable<std::unique_ptr<Move>> random_moves(Solution& s) = 0;
    virtual std::unique_ptr<Move> next(Solution& s) = 0;
};

class MoveGeneratorUnion : public MoveGenerator {
public:
    const Problem& problem;
    std::mt19937& gen;
    std::array<std::pair<MoveGenerator*, double>, 8>& mg_weights;
    
    std::array<MoveGenerator*, 8> mgs;
    MoveGenerator* move_generator_indices[100000];
    std::uniform_int_distribution<int> distr_mg_index;
    int move_generator_index = 0;

    MoveGeneratorUnion(const Problem& problem, std::mt19937& gen, std::array<std::pair<MoveGenerator*, double>, 8>& mg_weights);
    Iterable<std::unique_ptr<Move>> random_moves(Solution& s) override;
    std::unique_ptr<Move> next(Solution& s) override;
    void compute_weight_indices();
};

class MoveGeneratorSetPatient : public MoveGenerator {
public:
    const Problem& problem;
    std::mt19937& gen;

    int patient_index = 0;

    MoveGeneratorSetPatient(const Problem& problem, std::mt19937& gen)
        : problem(problem), gen(gen) {}
    Iterable<std::unique_ptr<Move>> random_moves(Solution& s) override;
    Iterable<MoveSetPatient> random_moves_definitive(Solution& s);
    std::unique_ptr<Move> next(Solution& s) override;
};


class MoveGeneratorSetNurse : public MoveGenerator {
public:
    const Problem& problem;
    std::mt19937& gen;

    int day_index = 0;

    MoveGeneratorSetNurse(const Problem& problem, std::mt19937& gen)
        : problem(problem), gen(gen) {}
    Iterable<std::unique_ptr<Move>> random_moves(Solution& s) override;
    Iterable<MoveSetNurse> random_moves_definitive(Solution& s);
    std::unique_ptr<Move> next(Solution& s) override;
};

class MoveGeneratorRemoveOptionalPatient : public MoveGenerator {
public:
    const Problem& problem;
    std::mt19937& gen;

    int patient_index = 0;

    MoveGeneratorRemoveOptionalPatient(const Problem& problem, std::mt19937& gen)
        : problem(problem), gen(gen) {}
    Iterable<std::unique_ptr<Move>> random_moves(Solution& s) override;
    Iterable<MoveSetPatient> random_moves_definitive(Solution& s);
    std::unique_ptr<Move> next(Solution& s) override;
};

class MoveGeneratorSwapPatients : public MoveGenerator {
public:
    const Problem& problem;
    std::mt19937& gen;

    int patient_index = 0;

    MoveGeneratorSwapPatients(const Problem& problem, std::mt19937& gen)
        : problem(problem), gen(gen) {}
    Iterable<std::unique_ptr<Move>> random_moves(Solution& s) override;
    Iterable<MoveChain> random_moves_definitive(Solution& s);
    std::unique_ptr<Move> next(Solution& s) override;
};

class MoveGeneratorKickPatient : public MoveGenerator {
public:
    const Problem& problem;
    std::mt19937& gen;

    int patient_index = 0;

    MoveGeneratorKickPatient(const Problem& problem, std::mt19937& gen)
        : problem(problem), gen(gen) {}
    Iterable<std::unique_ptr<Move>> random_moves(Solution& s) override;
    Iterable<MoveChain> random_moves_definitive(Solution& s);
    std::unique_ptr<Move> next(Solution& s) override;
};

class MoveGeneratorSwapNursesAll : public MoveGenerator {
public:
    const Problem& problem;
    std::mt19937& gen;

    int day_index = 0;

    MoveGeneratorSwapNursesAll(const Problem& problem, std::mt19937& gen)
        : problem(problem), gen(gen) {}
    Iterable<std::unique_ptr<Move>> random_moves(Solution& s) override;
    Iterable<MoveChain> random_moves_definitive(Solution& s);
    std::unique_ptr<Move> next(Solution& s) override;
};

class MoveGeneratorSwapNursesSingle : public MoveGenerator {
public:
    const Problem& problem;
    std::mt19937& gen;

    int day_index = 0;

    MoveGeneratorSwapNursesSingle(const Problem& problem, std::mt19937& gen)
        : problem(problem), gen(gen) {}
    Iterable<std::unique_ptr<Move>> random_moves(Solution& s) override;
    Iterable<MoveChain> random_moves_definitive(Solution& s);
    std::unique_ptr<Move> next(Solution& s) override;
};

class MoveGeneratorKickPatientOut : public MoveGenerator {
public:
    const Problem& problem;
    std::mt19937& gen;

    int patient_index = 0;

    MoveGeneratorKickPatientOut(const Problem& problem, std::mt19937& gen)
        : problem(problem), gen(gen) {}
    Iterable<std::unique_ptr<Move>> random_moves(Solution& s) override;
    Iterable<MoveChain> random_moves_definitive(Solution& s);
    std::unique_ptr<Move> next(Solution& s) override;
};


#endif
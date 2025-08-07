
#include <iostream>
#include <string>

#include "problem.h"
#include "solution_data.h"
#include "solution.h"
#include "move.h"
#include "move_generator.h"
#include "evaluation.h"
#include "perturbator.h"

#include "config.h"
#include "algorithms.h"
#include <future>

void run4(const Problem& p, Solution& s, std::mt19937& gen, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, int debug_level) {
    // 0315ba025bf511358f7cd2bc167b7bb6
    std::vector<double> _mg_weights = {0.5, 0.7, 0.7, 0.8, 0.5, 0.001, 0.1, 0.2};
    std::array<std::pair<MoveGenerator*, double>, 8> mg_weights_sa = {
        std::make_pair(new MoveGeneratorRemoveOptionalPatient(p, gen), _mg_weights[0]),
        std::make_pair(new MoveGeneratorSetPatient(p, gen), _mg_weights[1]),
        std::make_pair(new MoveGeneratorSetNurse(p, gen), _mg_weights[2]),
        std::make_pair(new MoveGeneratorSwapPatients(p, gen), _mg_weights[3]),
        std::make_pair(new MoveGeneratorSwapNursesSingle(p, gen), _mg_weights[4]),
        std::make_pair(new MoveGeneratorSwapNursesAll(p, gen), _mg_weights[5]),
        std::make_pair(new MoveGeneratorKickPatient(p, gen), _mg_weights[6]),
        std::make_pair(new MoveGeneratorKickPatientOut(p, gen), _mg_weights[7])
    };
    MoveGeneratorUnion mgu1(p, gen, mg_weights_sa);

    sa_plus(s, eval_elem_hard_constraints, eval_elem_soft_constraints, mgu1, gen, RUNTIME_ALG1 + RUNTIME_ALG2, {80, 25}, {0.2, 1}, debug_level);
}


void run3(const Problem& p, Solution& s, std::mt19937& gen, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, int debug_level) {
    // 0315ba025bf511358f7cd2bc167b7bb6
    std::vector<double> _mg_weights = {0.88, 0.56, 0.54, 0.932, 0.2, 0.0044, 0.3, 0.44};
    std::array<std::pair<MoveGenerator*, double>, 8> mg_weights_sa = {
        std::make_pair(new MoveGeneratorRemoveOptionalPatient(p, gen), _mg_weights[0]),
        std::make_pair(new MoveGeneratorSetPatient(p, gen), _mg_weights[1]),
        std::make_pair(new MoveGeneratorSetNurse(p, gen), _mg_weights[2]),
        std::make_pair(new MoveGeneratorSwapPatients(p, gen), _mg_weights[3]),
        std::make_pair(new MoveGeneratorSwapNursesSingle(p, gen), _mg_weights[4]),
        std::make_pair(new MoveGeneratorSwapNursesAll(p, gen), _mg_weights[5]),
        std::make_pair(new MoveGeneratorKickPatient(p, gen), _mg_weights[6]),
        std::make_pair(new MoveGeneratorKickPatientOut(p, gen), _mg_weights[7])
    };
    MoveGeneratorUnion mgu1(p, gen, mg_weights_sa);

    std::array<std::pair<MoveGenerator*, double>, 8> mg_weights_ils = {
        std::make_pair(new MoveGeneratorRemoveOptionalPatient(p, gen), _mg_weights[0]),
        std::make_pair(new MoveGeneratorSetPatient(p, gen), _mg_weights[1]),
        std::make_pair(new MoveGeneratorSetNurse(p, gen), _mg_weights[2]),
        std::make_pair(new MoveGeneratorSwapPatients(p, gen), _mg_weights[3]),
        std::make_pair(new MoveGeneratorSwapNursesSingle(p, gen), _mg_weights[4]),
        std::make_pair(new MoveGeneratorSwapNursesAll(p, gen), _mg_weights[5]),
        std::make_pair(new MoveGeneratorKickPatient(p, gen), _mg_weights[6]),
        std::make_pair(new MoveGeneratorKickPatientOut(p, gen), _mg_weights[7])
    };
    MoveGeneratorUnion mgu2(p, gen, mg_weights_ils);

    
    std::vector<std::pair<Perturbator*, double>> pu_weights = {};
    std::vector<double> _pu_weights = {};
    _pu_weights.assign(21, 1);
    
    std::unique_ptr<MoveGenerator> mg2 = std::make_unique<MoveGeneratorSetNurse>(p, gen);
    std::unique_ptr<MoveGenerator> mg3 = std::make_unique<MoveGeneratorSwapNursesSingle>(p, gen);
    std::unique_ptr<MoveGenerator> mg4 = std::make_unique<MoveGeneratorSwapNursesAll>(p, gen);
    std::unique_ptr<MoveGenerator> mg5 = std::make_unique<MoveGeneratorKickPatientOut>(p, gen);

    pu_weights.push_back(std::make_pair(new PerturbatorSetPatientsMany(p, gen, 3, 12), _pu_weights[0]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg2), p.nurses.size(), 0.1, 1), _pu_weights[1]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg3), p.nurses.size() / 2, 0.1, 1), _pu_weights[2]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg4), p.nurses.size() / 4, 0.1, 1), _pu_weights[3]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg5), p.optional_patient_ids.size(), 0.05, 0.4), _pu_weights[4]));
    pu_weights.push_back(std::make_pair(new PerturbatorRemoveOptionalPatients(p, gen, 0.03, 0.4), _pu_weights[5]));
    pu_weights.push_back(std::make_pair(new PerturbatorCloseRooms(p, gen, 0.3, 1), _pu_weights[6]));
    pu_weights.push_back(std::make_pair(new PerturbatorCloseOperatingTheaters(p, gen, 0.1, 0.8), _pu_weights[7]));
    pu_weights.push_back(std::make_pair(new PerturbatorCloseOperatingTheaterDays(p, gen, 0.55, 1), _pu_weights[8]));
    pu_weights.push_back(std::make_pair(new PerturbatorEnforceAdmissionDay(p, gen, 0.05, 0.22), _pu_weights[9]));
    pu_weights.push_back(std::make_pair(new PerturbatorPermutatePatientRooms(p, gen, 0.05, 0.4), _pu_weights[10]));
    pu_weights.push_back(std::make_pair(new PerturbatorPermutatePatientOperatingTheaters(p, gen, 0.05, 0.8), _pu_weights[11]));
    pu_weights.push_back(std::make_pair(new PerturbatorPermutatePatientAdmissionDays(p, gen, 0.05, 0.5), _pu_weights[12]));
    pu_weights.push_back(std::make_pair(new PerturbatorPatientAdmissionDayOffset(p, gen, -1, 0.05, 0.5), _pu_weights[13]));
    pu_weights.push_back(std::make_pair(new PerturbatorPatientAdmissionDayOffset(p, gen, 1, 0.05, 0.45), _pu_weights[14]));
    pu_weights.push_back(std::make_pair(new PerturbatorChangeWeights(p, gen, mgu2, eval_elem_hard_constraints, eval_elem_soft_constraints, 0.6, 0.95, 1), _pu_weights[15]));
    pu_weights.push_back(std::make_pair(new PerturbatorShuffleEquivalentNurses(p, gen, 0.05, 0.25), _pu_weights[16]));
    pu_weights.push_back(std::make_pair(new PerturbatorAssignUnscheduledPatientsBest(p, gen, 0.05, 0.4), _pu_weights[17]));
    if (p.surgeons.size() > 1) {
        pu_weights.push_back(std::make_pair(new PerturbatorSurgeonDayRemoveSingleOT(p, gen, 0.05, 0.5), _pu_weights[18]));
    }
    pu_weights.push_back(std::make_pair(new PerturbatorNursePatientShuffleLowCareCount(p, gen, 0.05, 0.35), _pu_weights[19]));
    pu_weights.push_back(std::make_pair(new PerturbatorNurseShuffleOverWorkload(p, gen, 0.05, 0.35), _pu_weights[20]));

    PerturbatorUnion pu = PerturbatorUnion(p, gen, pu_weights);

    sa_plus(s, eval_elem_hard_constraints, eval_elem_soft_constraints, mgu1, gen, RUNTIME_ALG1 - 120, {41, 24}, {0.2, 1}, debug_level);
    ils2(s, eval_elem_hard_constraints, eval_elem_soft_constraints, mgu2, pu, 0.3, gen, RUNTIME_ALG2 + 120, debug_level, 25);
}

void run2(const Problem& p, Solution& s, std::mt19937& gen, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, int debug_level) {
    // 3a43dbdf87883b899f4bf56e8d6d75fc
    std::vector<double> _mg_weights = {0.5, 0.7, 0.7, 0.8, 0.5, 0.001, 0.1, 0.2};

    std::array<std::pair<MoveGenerator*, double>, 8> mg_weights_sa = {
        std::make_pair(new MoveGeneratorRemoveOptionalPatient(p, gen), _mg_weights[0]),
        std::make_pair(new MoveGeneratorSetPatient(p, gen), _mg_weights[1]),
        std::make_pair(new MoveGeneratorSetNurse(p, gen), _mg_weights[2]),
        std::make_pair(new MoveGeneratorSwapPatients(p, gen), _mg_weights[3]),
        std::make_pair(new MoveGeneratorSwapNursesSingle(p, gen), _mg_weights[4]),
        std::make_pair(new MoveGeneratorSwapNursesAll(p, gen), _mg_weights[5]),
        std::make_pair(new MoveGeneratorKickPatient(p, gen), _mg_weights[6]),
        std::make_pair(new MoveGeneratorKickPatientOut(p, gen), _mg_weights[7])
    };
    MoveGeneratorUnion mgu1(p, gen, mg_weights_sa);

    std::array<std::pair<MoveGenerator*, double>, 8> mg_weights_ils = {
        std::make_pair(new MoveGeneratorRemoveOptionalPatient(p, gen), _mg_weights[0]),
        std::make_pair(new MoveGeneratorSetPatient(p, gen), _mg_weights[1]),
        std::make_pair(new MoveGeneratorSetNurse(p, gen), _mg_weights[2]),
        std::make_pair(new MoveGeneratorSwapPatients(p, gen), _mg_weights[3]),
        std::make_pair(new MoveGeneratorSwapNursesSingle(p, gen), _mg_weights[4]),
        std::make_pair(new MoveGeneratorSwapNursesAll(p, gen), _mg_weights[5]),
        std::make_pair(new MoveGeneratorKickPatient(p, gen), _mg_weights[6]),
        std::make_pair(new MoveGeneratorKickPatientOut(p, gen), _mg_weights[7])
    };
    MoveGeneratorUnion mgu2(p, gen, mg_weights_ils);

    
    std::vector<std::pair<Perturbator*, double>> pu_weights = {};
    std::vector<double> _pu_weights = {};
    _pu_weights.assign(21, 1);
    
    std::unique_ptr<MoveGenerator> mg2 = std::make_unique<MoveGeneratorSetNurse>(p, gen);
    std::unique_ptr<MoveGenerator> mg3 = std::make_unique<MoveGeneratorSwapNursesSingle>(p, gen);
    std::unique_ptr<MoveGenerator> mg4 = std::make_unique<MoveGeneratorSwapNursesAll>(p, gen);
    std::unique_ptr<MoveGenerator> mg5 = std::make_unique<MoveGeneratorKickPatientOut>(p, gen);

    pu_weights.push_back(std::make_pair(new PerturbatorSetPatientsMany(p, gen, 3, 12), _pu_weights[0]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg2), p.nurses.size(), 0.1, 1), _pu_weights[1]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg3), p.nurses.size() / 2, 0.1, 1), _pu_weights[2]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg4), p.nurses.size() / 4, 0.1, 1), _pu_weights[3]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg5), p.optional_patient_ids.size(), 0.05, 0.4), _pu_weights[4]));
    pu_weights.push_back(std::make_pair(new PerturbatorRemoveOptionalPatients(p, gen, 0.03, 0.4), _pu_weights[5]));
    pu_weights.push_back(std::make_pair(new PerturbatorCloseRooms(p, gen, 0.3, 1), _pu_weights[6]));
    pu_weights.push_back(std::make_pair(new PerturbatorCloseOperatingTheaters(p, gen, 0.1, 0.8), _pu_weights[7]));
    pu_weights.push_back(std::make_pair(new PerturbatorCloseOperatingTheaterDays(p, gen, 0.55, 1), _pu_weights[8]));
    pu_weights.push_back(std::make_pair(new PerturbatorEnforceAdmissionDay(p, gen, 0.05, 0.22), _pu_weights[9]));
    pu_weights.push_back(std::make_pair(new PerturbatorPermutatePatientRooms(p, gen, 0.05, 0.4), _pu_weights[10]));
    pu_weights.push_back(std::make_pair(new PerturbatorPermutatePatientOperatingTheaters(p, gen, 0.05, 0.8), _pu_weights[11]));
    pu_weights.push_back(std::make_pair(new PerturbatorPermutatePatientAdmissionDays(p, gen, 0.05, 0.5), _pu_weights[12]));
    pu_weights.push_back(std::make_pair(new PerturbatorPatientAdmissionDayOffset(p, gen, -1, 0.05, 0.5), _pu_weights[13]));
    pu_weights.push_back(std::make_pair(new PerturbatorPatientAdmissionDayOffset(p, gen, 1, 0.05, 0.45), _pu_weights[14]));
    pu_weights.push_back(std::make_pair(new PerturbatorChangeWeights(p, gen, mgu2, eval_elem_hard_constraints, eval_elem_soft_constraints, 0.6, 0.95, 1), _pu_weights[15]));
    pu_weights.push_back(std::make_pair(new PerturbatorShuffleEquivalentNurses(p, gen, 0.05, 0.25), _pu_weights[16]));
    pu_weights.push_back(std::make_pair(new PerturbatorAssignUnscheduledPatientsBest(p, gen, 0.05, 0.4), _pu_weights[17]));
    if (p.surgeons.size() > 1) {
        pu_weights.push_back(std::make_pair(new PerturbatorSurgeonDayRemoveSingleOT(p, gen, 0.05, 0.5), _pu_weights[18]));
    }
    pu_weights.push_back(std::make_pair(new PerturbatorNursePatientShuffleLowCareCount(p, gen, 0.05, 0.35), _pu_weights[19]));
    pu_weights.push_back(std::make_pair(new PerturbatorNurseShuffleOverWorkload(p, gen, 0.05, 0.35), _pu_weights[20]));

    PerturbatorUnion pu = PerturbatorUnion(p, gen, pu_weights);

    sa_plus(s, eval_elem_hard_constraints, eval_elem_soft_constraints, mgu1, gen, RUNTIME_ALG1, {80, 25}, {0.3, 1}, debug_level);
    ils2(s, eval_elem_hard_constraints, eval_elem_soft_constraints, mgu2, pu, 0.3, gen, RUNTIME_ALG2, debug_level, 25);
}

void run1(const Problem& p, Solution& s, std::mt19937& gen, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, int debug_level) {
    // 275d231b131b5063bc52336d7af296ed
    std::vector<double> _mg_weights = {0.5, 0.7, 0.7, 0.8, 0.5, 0.001, 0.1, 0.2};

    std::array<std::pair<MoveGenerator*, double>, 8> mg_weights_sa = {
        std::make_pair(new MoveGeneratorRemoveOptionalPatient(p, gen), _mg_weights[0]),
        std::make_pair(new MoveGeneratorSetPatient(p, gen), _mg_weights[1]),
        std::make_pair(new MoveGeneratorSetNurse(p, gen), _mg_weights[2]),
        std::make_pair(new MoveGeneratorSwapPatients(p, gen), _mg_weights[3]),
        std::make_pair(new MoveGeneratorSwapNursesSingle(p, gen), _mg_weights[4]),
        std::make_pair(new MoveGeneratorSwapNursesAll(p, gen), _mg_weights[5]),
        std::make_pair(new MoveGeneratorKickPatient(p, gen), _mg_weights[6]),
        std::make_pair(new MoveGeneratorKickPatientOut(p, gen), _mg_weights[7])
    };
    MoveGeneratorUnion mgu1(p, gen, mg_weights_sa);

    std::array<std::pair<MoveGenerator*, double>, 8> mg_weights_ils = {
        std::make_pair(new MoveGeneratorRemoveOptionalPatient(p, gen), _mg_weights[0]),
        std::make_pair(new MoveGeneratorSetPatient(p, gen), _mg_weights[1]),
        std::make_pair(new MoveGeneratorSetNurse(p, gen), _mg_weights[2]),
        std::make_pair(new MoveGeneratorSwapPatients(p, gen), _mg_weights[3]),
        std::make_pair(new MoveGeneratorSwapNursesSingle(p, gen), _mg_weights[4]),
        std::make_pair(new MoveGeneratorRemoveOptionalPatient(p, gen), _mg_weights[5]),
        //std::make_pair(new MoveGeneratorSwapNursesAll(p, gen), _mg_weights[5]),
        std::make_pair(new MoveGeneratorKickPatient(p, gen), _mg_weights[6]),
        std::make_pair(new MoveGeneratorKickPatientOut(p, gen), _mg_weights[7])
    };
    MoveGeneratorUnion mgu2(p, gen, mg_weights_ils);

    
    std::vector<std::pair<Perturbator*, double>> pu_weights = {};
    std::vector<double> _pu_weights = {};
    _pu_weights.assign(21, 1);
    
    std::unique_ptr<MoveGenerator> mg2 = std::make_unique<MoveGeneratorSetNurse>(p, gen);
    std::unique_ptr<MoveGenerator> mg3 = std::make_unique<MoveGeneratorSwapNursesSingle>(p, gen);
    std::unique_ptr<MoveGenerator> mg4 = std::make_unique<MoveGeneratorSwapNursesAll>(p, gen);
    std::unique_ptr<MoveGenerator> mg5 = std::make_unique<MoveGeneratorKickPatientOut>(p, gen);

    pu_weights.push_back(std::make_pair(new PerturbatorSetPatientsMany(p, gen, 3, 12), _pu_weights[0]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg2), p.nurses.size(), 0.1, 1), _pu_weights[1]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg3), p.nurses.size() / 2, 0.1, 1), _pu_weights[2]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg4), p.nurses.size() / 4, 0.1, 1), _pu_weights[3]));
    pu_weights.push_back(std::make_pair(new PerturbatorFromGenerator(p, std::move(mg5), p.optional_patient_ids.size(), 0.05, 0.4), _pu_weights[4]));
    pu_weights.push_back(std::make_pair(new PerturbatorRemoveOptionalPatients(p, gen, 0.03, 0.4), _pu_weights[5]));
    pu_weights.push_back(std::make_pair(new PerturbatorCloseRooms(p, gen, 0.3, 1), _pu_weights[6]));
    pu_weights.push_back(std::make_pair(new PerturbatorCloseOperatingTheaters(p, gen, 0.1, 0.8), _pu_weights[7]));
    pu_weights.push_back(std::make_pair(new PerturbatorCloseOperatingTheaterDays(p, gen, 0.55, 1), _pu_weights[8]));
    pu_weights.push_back(std::make_pair(new PerturbatorEnforceAdmissionDay(p, gen, 0.05, 0.22), _pu_weights[9]));
    pu_weights.push_back(std::make_pair(new PerturbatorPermutatePatientRooms(p, gen, 0.05, 0.4), _pu_weights[10]));
    pu_weights.push_back(std::make_pair(new PerturbatorPermutatePatientOperatingTheaters(p, gen, 0.05, 0.8), _pu_weights[11]));
    pu_weights.push_back(std::make_pair(new PerturbatorPermutatePatientAdmissionDays(p, gen, 0.05, 0.5), _pu_weights[12]));
    pu_weights.push_back(std::make_pair(new PerturbatorPatientAdmissionDayOffset(p, gen, -1, 0.05, 0.5), _pu_weights[13]));
    pu_weights.push_back(std::make_pair(new PerturbatorPatientAdmissionDayOffset(p, gen, 1, 0.05, 0.45), _pu_weights[14]));
    pu_weights.push_back(std::make_pair(new PerturbatorChangeWeights(p, gen, mgu2, eval_elem_hard_constraints, eval_elem_soft_constraints, 0.6, 0.95, 1), _pu_weights[15]));
    pu_weights.push_back(std::make_pair(new PerturbatorShuffleEquivalentNurses(p, gen, 0.05, 0.25), _pu_weights[16]));
    pu_weights.push_back(std::make_pair(new PerturbatorAssignUnscheduledPatientsBest(p, gen, 0.05, 0.4), _pu_weights[17]));
    if (p.surgeons.size() > 1) {
        pu_weights.push_back(std::make_pair(new PerturbatorSurgeonDayRemoveSingleOT(p, gen, 0.05, 0.5), _pu_weights[18]));
    }
    pu_weights.push_back(std::make_pair(new PerturbatorNursePatientShuffleLowCareCount(p, gen, 0.05, 0.35), _pu_weights[19]));
    pu_weights.push_back(std::make_pair(new PerturbatorNurseShuffleOverWorkload(p, gen, 0.05, 0.35), _pu_weights[20]));

    PerturbatorUnion pu = PerturbatorUnion(p, gen, pu_weights);

    sa_plus(s, eval_elem_hard_constraints, eval_elem_soft_constraints, mgu1, gen, RUNTIME_ALG1, {80, 25}, {0.3, 1}, debug_level);
    ils(s, eval_elem_hard_constraints, eval_elem_soft_constraints, mgu2, pu, 0.3, gen, RUNTIME_ALG2, debug_level, 25);
}

int main(int argc, char **argv) {
    // ----------------
    // Problem
    // ----------------
    if (argc < 1) {
        std::cout << "Usage: " << argv[0] << " <problem_file=data/../i01.json> <seed={random}> <out_file=out/sol_[].json> <debug_level=0>" << std::endl;
        return 1;
    }

    std::string filename = "data/ihtc2024_competition_dataset/i01.json";
    if (argc >= 2) {
        filename = argv[1];
    }

    std::random_device rd = std::random_device();
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    if (argc >= 3) {
        seed = std::stoi(argv[2]);
    }
    std::mt19937 gen = std::mt19937(seed);
    std::cout << "Using seed: " << seed << std::endl;

    std::string instance_name = filename.substr(filename.find_last_of("/") + 1, filename.find_last_of(".") - filename.find_last_of("/"));
    std::string filename_out = "out/sol_" + instance_name + "json";
    if (argc >= 4) {
        filename_out = argv[3];
    }
    std::cout << "Output file: " << filename_out << std::endl;

    int debug_level = 0;
    if (argc >= 5) {
        debug_level = std::stoi(argv[4]);
    }
    std::cout << "Debug level: " << debug_level << std::endl;

    //filename = "../data/ihtc2024_competition_dataset/i01.json";
    Problem p = Problem::from_textio(filename, gen);

    EvalElemHardConstraints eval_elem_hard_constraints = EvalElemHardConstraints(p);


    // 1
    std::random_device rd1 = std::random_device();
    auto dist1 = std::uniform_int_distribution<unsigned>(0, 10000);
    unsigned seed1 = dist1(rd1);
    std::mt19937 gen1 = std::mt19937(seed1);
    SolutionData sd1 = SolutionData::random_initial_solution(p, gen1);
    Solution s1 = Solution(p, sd1);
    s1.eval_elems.push_back(std::make_unique<EvalElemHardConstraints>(p));
    s1.eval_elems.push_back(std::make_unique<EvalElemSoftConstraints>(p, 
        p.weights["open_operating_theater"], 
        p.weights["room_mixed_age"],
        p.weights["patient_delay"],
        p.weights["unscheduled_optional"],
        p.weights["surgeon_transfer"],
        p.weights["room_nurse_skill"],
        p.weights["continuity_of_care"],
        p.weights["nurse_eccessive_workload"])
    );
    EvalElemSoftConstraints eval_elem_soft_constraints1 = EvalElemSoftConstraints(p, 
        p.weights["open_operating_theater"], 
        p.weights["room_mixed_age"],
        p.weights["patient_delay"],
        p.weights["unscheduled_optional"],
        p.weights["surgeon_transfer"],
        p.weights["room_nurse_skill"],
        p.weights["continuity_of_care"],
        p.weights["nurse_eccessive_workload"]);
    s1.evaluation = s1.get_objective(false);




    // 2
    std::random_device rd2 = std::random_device();
    auto dist2 = std::uniform_int_distribution<unsigned>(0, 10000);
    unsigned seed2 = dist2(rd2);
    std::mt19937 gen2 = std::mt19937(seed2);
    SolutionData sd2 = SolutionData::random_initial_solution(p, gen2);
    Solution s2 = Solution(p, sd2);
    s2.eval_elems.push_back(std::make_unique<EvalElemHardConstraints>(p));
    s2.eval_elems.push_back(std::make_unique<EvalElemSoftConstraints>(p, 
        p.weights["open_operating_theater"], 
        p.weights["room_mixed_age"],
        p.weights["patient_delay"],
        p.weights["unscheduled_optional"],
        p.weights["surgeon_transfer"],
        p.weights["room_nurse_skill"],
        p.weights["continuity_of_care"],
        p.weights["nurse_eccessive_workload"])
    );
    EvalElemSoftConstraints eval_elem_soft_constraints2 = EvalElemSoftConstraints(p, 
        p.weights["open_operating_theater"], 
        p.weights["room_mixed_age"],
        p.weights["patient_delay"],
        p.weights["unscheduled_optional"],
        p.weights["surgeon_transfer"],
        p.weights["room_nurse_skill"],
        p.weights["continuity_of_care"],
        p.weights["nurse_eccessive_workload"]);
    s2.evaluation = s2.get_objective(false);

    // 3
    std::random_device rd3 = std::random_device();
    auto dist3 = std::uniform_int_distribution<unsigned>(0, 10000);
    unsigned seed3 = dist3(rd3);
    std::mt19937 gen3 = std::mt19937(seed3);
    SolutionData sd3 = SolutionData::random_initial_solution(p, gen3);
    Solution s3 = Solution(p, sd3);
    s3.eval_elems.push_back(std::make_unique<EvalElemHardConstraints>(p));
    s3.eval_elems.push_back(std::make_unique<EvalElemSoftConstraints>(p, 
        p.weights["open_operating_theater"], 
        p.weights["room_mixed_age"],
        p.weights["patient_delay"],
        p.weights["unscheduled_optional"],
        p.weights["surgeon_transfer"],
        p.weights["room_nurse_skill"],
        p.weights["continuity_of_care"],
        p.weights["nurse_eccessive_workload"])
    );
    EvalElemSoftConstraints eval_elem_soft_constraints3 = EvalElemSoftConstraints(p, 
        p.weights["open_operating_theater"], 
        p.weights["room_mixed_age"],
        p.weights["patient_delay"],
        p.weights["unscheduled_optional"],
        p.weights["surgeon_transfer"],
        p.weights["room_nurse_skill"],
        p.weights["continuity_of_care"],
        p.weights["nurse_eccessive_workload"]);
    s3.evaluation = s3.get_objective(false);


    // 4
    std::random_device rd4 = std::random_device();
    auto dist4 = std::uniform_int_distribution<unsigned>(0, 10000);
    unsigned seed4 = dist4(rd4);
    std::mt19937 gen4 = std::mt19937(seed4);
    SolutionData sd4 = SolutionData::random_initial_solution(p, gen4);
    Solution s4 = Solution(p, sd4);
    s4.eval_elems.push_back(std::make_unique<EvalElemHardConstraints>(p));
    s4.eval_elems.push_back(std::make_unique<EvalElemSoftConstraints>(p, 
        p.weights["open_operating_theater"], 
        p.weights["room_mixed_age"],
        p.weights["patient_delay"],
        p.weights["unscheduled_optional"],
        p.weights["surgeon_transfer"],
        p.weights["room_nurse_skill"],
        p.weights["continuity_of_care"],
        p.weights["nurse_eccessive_workload"])
    );
    EvalElemSoftConstraints eval_elem_soft_constraints4 = EvalElemSoftConstraints(p, 
        p.weights["open_operating_theater"], 
        p.weights["room_mixed_age"],
        p.weights["patient_delay"],
        p.weights["unscheduled_optional"],
        p.weights["surgeon_transfer"],
        p.weights["room_nurse_skill"],
        p.weights["continuity_of_care"],
        p.weights["nurse_eccessive_workload"]);
    s4.evaluation = s4.get_objective(false);    

    
    std::vector<std::future<void>> futures;
    futures.push_back(std::async(std::launch::async, run1, std::ref(p), std::ref(s1), std::ref(gen1), std::ref(eval_elem_hard_constraints), std::ref(eval_elem_soft_constraints1), debug_level));
    futures.push_back(std::async(std::launch::async, run2, std::ref(p), std::ref(s2), std::ref(gen2), std::ref(eval_elem_hard_constraints), std::ref(eval_elem_soft_constraints2), debug_level));
    futures.push_back(std::async(std::launch::async, run3, std::ref(p), std::ref(s3), std::ref(gen3), std::ref(eval_elem_hard_constraints), std::ref(eval_elem_soft_constraints3), debug_level));

    run4(p, s4, gen4, eval_elem_hard_constraints, eval_elem_soft_constraints4, debug_level);

    for (auto& f : futures) {
        f.get();
    }
    
    s1.evaluation = s1.get_objective(false);
    s2.evaluation = s2.get_objective(false);
    s3.evaluation = s3.get_objective(false);
    s4.evaluation = s4.get_objective(false);
    std::cout << "s1 evaluation: " << s1.evaluation.to_string() << std::endl;
    std::cout << "s2 evaluation: " << s2.evaluation.to_string() << std::endl;
    std::cout << "s3 evaluation: " << s3.evaluation.to_string() << std::endl;
    std::cout << "s4 evaluation: " << s4.evaluation.to_string() << std::endl;

    Evaluation best_eval = s1.evaluation;
    SolutionData best_sd = s1.sd;
    int which = 1;
    if (s2.evaluation.values[0] <= best_eval.values[0]
        && s2.evaluation.values[s2.evaluation.values.size() - 1] < best_eval.values[best_eval.values.size() - 1]) {
        best_eval = s2.evaluation;
        best_sd = s2.sd;
        which = 2;
    }
    if (s3.evaluation.values[0] <= best_eval.values[0]
        && s3.evaluation.values[s3.evaluation.values.size() - 1] < best_eval.values[best_eval.values.size() - 1])
    {
        best_eval = s3.evaluation;
        best_sd = s3.sd;
        which = 3;
    }
    if (s4.evaluation.values[0] <= best_eval.values[0]
        && s4.evaluation.values[s4.evaluation.values.size() - 1] < best_eval.values[best_eval.values.size() - 1])
    {
        best_eval = s4.evaluation;
        best_sd = s4.sd;
        which = 4;
    }

    best_sd.write_to_file(filename_out);
    std::cout << "Saved to " << filename_out << std::endl;

    int sum_hardconstrs = 0;
    for (int i = 0; i < best_eval.values.size() - 1; i++) {
        sum_hardconstrs += best_eval.values[i];
    }
    if (sum_hardconstrs > 0) {
        std::cout << 999999 << std::endl;
    } else {
        std::cout << best_eval.values[best_eval.values.size() - 1] << std::endl;
    }
}
#ifndef EVALUATION_ELEMENT_H
#define EVALUATION_ELEMENT_H

class EvaluationElement;

#include <iostream>
#include <memory>

#include "solution_data.h"
#include "move.h"
#include "problem.h"

class EvalElemHardConstraints : public EvaluationElement {
    public:
        const Problem& problem;
        EvalElemHardConstraints(const Problem& problem) : problem(problem) {}
    
        int get_objective(SolutionData& sd, bool is_debug) override;
        int get_incr(SolutionData& sd, MoveSetPatient& move, bool is_commit) override;
        int get_incr(SolutionData& sd, MoveSetNurse& move, bool is_commit) override;
};

class EvalElemSoftConstraints : public EvaluationElement {
    public:
        const Problem& problem;
        int weight_OpenOperatingTheater;
        int weight_RoomAgeMix;
        int weight_PatientDelay;
        int weight_ElectiveUnscheduledPatients;
        int weight_SurgeonTransfer;
        int weight_RoomSkillLevel;
        int weight_ContinuityOfCare;
        int weight_ExcessiveNurseWorkload;

        EvalElemSoftConstraints(const Problem& problem, int weight_OpenOperatingTheater, int weight_RoomAgeMix, int weight_PatientDelay, int weight_ElectiveUnscheduledPatients, int weight_SurgeonTransfer, int weight_RoomSkillLevel, int weight_ContinuityOfCare, int weight_ExcessiveNurseWorkload) : problem(problem), weight_OpenOperatingTheater(weight_OpenOperatingTheater), weight_RoomAgeMix(weight_RoomAgeMix), weight_PatientDelay(weight_PatientDelay), weight_ElectiveUnscheduledPatients(weight_ElectiveUnscheduledPatients), weight_SurgeonTransfer(weight_SurgeonTransfer), weight_RoomSkillLevel(weight_RoomSkillLevel), weight_ContinuityOfCare(weight_ContinuityOfCare), weight_ExcessiveNurseWorkload(weight_ExcessiveNurseWorkload) {}
    
        std::vector<int> delta = std::vector<int>(problem.nurses.size(), 0);
        
        int get_objective(SolutionData& sd, bool is_debug) override;
        int get_incr(SolutionData& sd, MoveSetPatient& move, bool is_commit) override;
        int get_incr(SolutionData& sd, MoveSetNurse& move, bool is_commit) override;
};

#endif
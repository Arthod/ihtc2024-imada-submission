#include "move_generator.h"
#include "move.h"
#include "perturbator.h"

void sa_plus(Solution& s, 
    EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints,
    MoveGeneratorUnion& mg, std::mt19937& gen, int duration, 
    std::vector<double> temperatures, std::vector<double> temperature_duration_fractions, int debug_level) {
    
    auto start = std::chrono::system_clock::now();

    SolutionData best_sd = s.sd;
    Evaluation best_eval = s.evaluation;

    std::uniform_real_distribution<double> rand = std::uniform_real_distribution<double>(0, 1);

    while (true) {
        double time_elapsed = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
        if (time_elapsed >= duration) {
            break;
        }
        while (true) {
            auto move = mg.next(s);
            if (move == nullptr) {
                continue;
            }

            // Hard constraints
            int hard_sum_incr = move->get_incr(eval_elem_hard_constraints, s.sd, false);
            if (hard_sum_incr < 0) {
                s.step(*move);
                break;
            }
            if (hard_sum_incr > 0) {
                double time_elapsed_frac1 = time_elapsed / (double) (duration * temperature_duration_fractions[0]);
                int temp_max = temperatures[0];
                double temp = (double) temp_max * (1 - time_elapsed_frac1); // Linear cooling
                if (temp <= 0) {
                    break;
                }
                if (exp((double) -hard_sum_incr / temp) > rand(gen)) {
                    s.step(*move);
#if DEBUG >= 1
                    if (debug_level > 1) {
                        std::cout << time_elapsed << ", SA: Evaluation: " << s.evaluation.to_string() << ", Incr: " << hard_sum_incr << ", temp: " << temp << std::endl;
                    }
#endif
                }
                break;
            }

            // Soft constraints
            int soft_sum_incr = move->get_incr(eval_elem_soft_constraints, s.sd, false);
            if (soft_sum_incr <= 0) {
                s.step(*move);
                break;
            }
            double time_elapsed_frac2 = time_elapsed / (double) (duration * temperature_duration_fractions[1]);
            int temp_max = temperatures[1];
            double temp = (double) temp_max * (1 - time_elapsed_frac2); // Linear cooling
            if (temp <= 0) {
                break;
            }
            if (exp((double) -soft_sum_incr / temp) > rand(gen)) {
                s.step(*move);
#if DEBUG >= 1
                if (debug_level > 1) {
                    std::cout << time_elapsed << ", SA: Evaluation: " << s.evaluation.to_string() << ", Incr: " << soft_sum_incr << ", temp: " << temp << std::endl;
                }
#endif
                break;
            }
        }

        if (s.evaluation < best_eval) {
            best_sd = s.sd;
            best_eval = s.evaluation;
#if DEBUG >= 1
            if (debug_level > 0) {
                std::cout << time_elapsed << "SA: New best: " << s.evaluation.to_string() << std::endl;
            }
#endif
        }
        if (time_elapsed >= duration) {
            break;
        }
    }

    s.sd = best_sd;
    s.evaluation = best_eval;

#if DEBUG >= 1
    if (debug_level > 0) {
        std::cout << "SA: Best: " << s.evaluation.to_string() << ", returning best." << std::endl;
    }
#endif
}




void fi_plus(Solution& s, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, MoveGenerator& mg, std::mt19937& gen, int duration, int debug_level) {

    auto start = std::chrono::system_clock::now();
    
    while (std::chrono::system_clock::now() - start <= std::chrono::duration<double>(duration)) {
        
        bool reached_local_optimum = true;
        auto gen = mg.random_moves(s);

        while (gen.move_next()) {
            auto move = gen.current();

            // Hard constraints
            int hard_sum_incr = move->get_incr(eval_elem_hard_constraints, s.sd, false);
            if (hard_sum_incr < 0) {
                s.step(*move);
#if DEBUG >= 1
                if (debug_level > 0) { 
                    std::cout << "FI: Evaluation: " << s.evaluation.to_string() << ", Incr: [" << hard_sum_incr << ", *]" << std::endl;
                }
#endif
                reached_local_optimum = false;
                break;
            }
            // Soft constraints
            if (hard_sum_incr == 0) {
                int soft_sum_incr = move->get_incr(eval_elem_soft_constraints, s.sd, false);
                if (soft_sum_incr < 0) {
                    s.step(*move);
#if DEBUG >= 1
                    if (debug_level > 0) { 
                        std::cout << "FI: Evaluation: " << s.evaluation.to_string() << ", Incr: [" << hard_sum_incr << ", " << soft_sum_incr << "]" << std::endl;
                    }
#endif
                    reached_local_optimum = false;
                    break;
                }
            }
        }

        if (reached_local_optimum) {
#if DEBUG >= 1
            if (debug_level > 0) { 
                std::cout << "FI: Reached local optimum" << std::endl;
            }
#endif
            break;
        }
    }
}



void fi_randomsubset_plus(Solution& s, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, MoveGenerator& mg, std::mt19937& gen, int duration, int subset_size, int debug_level) {

    auto start = std::chrono::system_clock::now();

    while (std::chrono::system_clock::now() - start <= std::chrono::duration<double>(duration)) {
        bool reached_local_optimum = true;

        for (int i = 0; i < subset_size; i++) {
            auto move = mg.next(s);
            if (move == nullptr) {
                continue;
            }

            // Hard constraints
            int hard_sum_incr = move->get_incr(eval_elem_hard_constraints, s.sd, false);
            if (hard_sum_incr < 0) {
                s.step(*move);
#if DEBUG >= 1
                if (debug_level > 0) { 
                    std::cout << "FIRS: Evaluation: " << s.evaluation.to_string() << ", Incr: [" << hard_sum_incr << ", *]" << std::endl;
                }
#endif
                reached_local_optimum = false;
                break;
            }

            // Soft constraints
            if (s.evaluation.values[0] == 0 && hard_sum_incr <= 0) {
                int soft_sum_incr = move->get_incr(eval_elem_soft_constraints, s.sd, false);
                if (soft_sum_incr < 0) {
                    s.step(*move);
#if DEBUG >= 1
                    if (debug_level > 0) { 
                        std::cout << "FIRS: Evaluation: " << s.evaluation.to_string() << ", Incr: [" << hard_sum_incr << ", " << soft_sum_incr << "]" << std::endl;
                    }
#endif
                    reached_local_optimum = false;
                    break;
                }
            }
        }

        if (reached_local_optimum) {
            if (debug_level > 0) { 
                std::cout << "FI-RSN: Reached local optimum" << std::endl;
            }
            break;
        }
    }
}


void ils2(Solution& s, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, MoveGeneratorUnion& mgu, PerturbatorUnion& pu, double ks, std::mt19937& gen, int duration, int debug_level, int temptol) {
    auto start = std::chrono::system_clock::now();

    SolutionData best_sd = s.sd;
    Evaluation best_eval = s.evaluation;

    SolutionData current_sd = s.sd;
    Evaluation current_eval = s.evaluation;

    int local_optimas = 0;
    int perturb_index = 0;
    
    double ks2 = 0;
    auto dist = std::uniform_real_distribution<double>(0, 1);
    while (std::chrono::system_clock::now() - start <= std::chrono::duration<double>(duration)) {
        int duration_remaining = (std::chrono::duration<double>(duration) - std::chrono::duration<double>(std::chrono::system_clock::now() - start)).count();
        fi_plus(s, eval_elem_hard_constraints, eval_elem_soft_constraints, mgu, gen, duration_remaining, debug_level - 3);

        if (s.evaluation <= best_eval) {
            bool is_strictlybetter = s.evaluation < best_eval;
            bool is_better = s.evaluation <= best_eval;
            if (is_better) {
                best_sd = s.sd;
                best_eval = s.evaluation;
                current_sd = s.sd;
                current_eval = s.evaluation;
            }
#if DEBUG >= 1
            if (debug_level > 0 && is_strictlybetter) {
                std::cout << duration_remaining << " - ILS2: NewB:" << best_eval.to_string() << ", Perturb:" << pu.pus[perturb_index].get()->name() << "(ks=" << ks2 << "), local optimas: " << local_optimas << std::endl;
            }
#endif
            if (is_strictlybetter) {
                local_optimas = 0;
            }
        } else if (s.evaluation.values[0] <= best_eval.values[0] && s.evaluation.values[1] <= ((double) best_eval.values[1]) + temptol) {
            current_sd = s.sd;
            current_eval = s.evaluation;
            
#if DEBUG >= 1
            if (debug_level > 1) {
                std::cout << duration_remaining << " - ILS2: Side:" << best_eval.to_string() << ", C:" << s.evaluation.to_string() << ", sidestep Perturb:" << pu.pus[perturb_index].get()->name() << "(ks=" << ks2 << "), local optimas: " << local_optimas << std::endl;
            }
#endif
        } else {
            s.sd = current_sd;
            s.evaluation = current_eval;
            local_optimas++;
#if DEBUG >= 1
            if (debug_level > 2) {
                std::cout << duration_remaining << " - ILS2: Rjct:" << best_eval.to_string() << ", Perturb:" << pu.pus[perturb_index].get()->name() << "(ks=" << ks2 << "), local optimas: " << local_optimas << std::endl;
            }
#endif
        }

        ks2 = dist(gen);
        perturb_index = pu.perturb(s, ks2);

#if DEBUG >= 1
        if (debug_level > 3) {
            std::cout << "ILS2: Perturbed: " << s.evaluation.to_string() << std::endl;
        }
#endif
    }

    s.sd = best_sd;
    s.evaluation = best_eval;

#if DEBUG >= 1
    if (debug_level > 0) {
        std::cout << "ILS2: Best: " << s.evaluation.to_string() << ", returning best." << std::endl;
    }
#endif
}

void ils(Solution& s, EvalElemHardConstraints& eval_elem_hard_constraints, EvalElemSoftConstraints& eval_elem_soft_constraints, MoveGenerator& mg, PerturbatorUnion& pu, double ks, std::mt19937& gen, int duration, int debug_level, int temptol) {
    auto start = std::chrono::system_clock::now();

    SolutionData best_sd = s.sd;
    Evaluation best_eval = s.evaluation;

    int local_optimas = 0;
    int perturb_index = 0;
    
    double ks2 = 0;
    auto dist = std::uniform_real_distribution<double>(0, 1);
    while (std::chrono::system_clock::now() - start <= std::chrono::duration<double>(duration)) {
        int duration_remaining = (std::chrono::duration<double>(duration) - std::chrono::duration<double>(std::chrono::system_clock::now() - start)).count();
        fi_plus(s, eval_elem_hard_constraints, eval_elem_soft_constraints, mg, gen, duration_remaining, debug_level - 3);

        if (s.evaluation <= best_eval) {
            bool is_strictlybetter = s.evaluation < best_eval;
            bool is_better = s.evaluation <= best_eval;
            if (is_better) {
                best_sd = s.sd;
                best_eval = s.evaluation;
            }
#if DEBUG >= 1
            if (debug_level > 0 && is_strictlybetter) {
                std::cout << duration_remaining << " - ILS: NewB:" << best_eval.to_string() << ", Perturb:" << pu.pus[perturb_index].get()->name() << "(ks=" << ks2 << "), local optimas: " << local_optimas << std::endl;
            }
#endif
            if (is_strictlybetter) {
                local_optimas = 0;
            }
            

#if DEBUG >= 1
        } else if (s.evaluation.values[0] <= best_eval.values[0] && s.evaluation.values[1] <= best_eval.values[1] + temptol) {
            if (debug_level > 1) {
                std::cout << duration_remaining << " - ILS: Side:" << best_eval.to_string() << ", C:" << s.evaluation.to_string() << ", sidestep Perturb:" << pu.pus[perturb_index].get()->name() << "(ks=" << ks2 << "), local optimas: " << local_optimas << std::endl;
            }
#endif
        } else {
            s.sd = best_sd;
            s.evaluation = best_eval;
            local_optimas++;
#if DEBUG >= 1
            if (debug_level > 2) {
                std::cout << duration_remaining << " - ILS: Rjct:" << best_eval.to_string() << ", Perturb:" << pu.pus[perturb_index].get()->name() << "(ks=" << ks2 << "), local optimas: " << local_optimas << std::endl;
            }
#endif
        }

        ks2 = dist(gen);
        perturb_index = pu.perturb(s, ks2);

#if DEBUG >= 1
        if (debug_level > 3) {
            std::cout << "ILS: Perturbed: " << s.evaluation.to_string() << std::endl;
        }
#endif
    }

    s.sd = best_sd;
    s.evaluation = best_eval;

#if DEBUG >= 1
    if (debug_level > 0) {
        std::cout << "ILS: Best: " << s.evaluation.to_string() << ", returning best." << std::endl;
    }
#endif
}

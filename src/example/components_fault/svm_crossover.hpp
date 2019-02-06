//
// Created by marcin on 11/01/19.
//

#ifndef GENETIC_ACTOR_SVM_CROSSOVER_H
#define GENETIC_ACTOR_SVM_CROSSOVER_H

#include <core.hpp>
#include "components_fault_defs.hpp"

namespace cpga {
using namespace core;
namespace examples {
/**
 * @brief Crossover genetic operator for the component fault prediction problem.
 * @details This class applies a single-point crossover to a pair of individuals. Each pair of parents
 * (p1, p2) produces two offspring such that o = (p1.c, p2.gamma) or (p2.c, p1.gamma).
 */
class svm_crossover : public base_operator {
 private:
  std::default_random_engine generator;
  std::bernoulli_distribution distribution;
  std::function<bool()> coin_toss;
 public:
  svm_crossover() = default;
  svm_crossover(const shared_config &config, island_id island_no);

  void operator()(inserter<rbf_params, double> it, const wrapper_pair<rbf_params, double> &couple);
};
}
}

#endif //GENETIC_ACTOR_SVM_CROSSOVER_H

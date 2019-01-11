//
// Created by marcin on 11/01/19.
//

#ifndef GENETIC_ACTOR_SVM_CROSSOVER_H
#define GENETIC_ACTOR_SVM_CROSSOVER_H

#include <core.hpp>
#include "components_fault_defs.hpp"

class svm_crossover : public base_operator<rbf_params, double> {
  INCLUDES(rbf_params, double);
 private:
  std::default_random_engine generator;
  std::bernoulli_distribution distribution;
  std::function<bool()> coin_toss;
 public:
  svm_crossover(const shared_config &config, island_id island_no);

  void operator()(inserter it, const wrapper_pair &couple);
};

#endif //GENETIC_ACTOR_SVM_CROSSOVER_H

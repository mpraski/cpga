//
// Created by marcin on 11/01/19.
//

#ifndef GENETIC_ACTOR_SVM_MUTATION_H
#define GENETIC_ACTOR_SVM_MUTATION_H

#include <core.hpp>
#include "components_fault_defs.hpp"

class svm_mutation : public base_operator {
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> dist_mutate;
  std::uniform_real_distribution<double> dist_mutate_c;
  std::uniform_real_distribution<double> dist_mutate_gamma;

  static inline auto make_range(double a) {
    return std::uniform_real_distribution<double>{-a, a};
  }
 public:
  svm_mutation() = default;
  svm_mutation(const shared_config &config, island_id island_no);

  void operator()(wrapper<rbf_params, double> &wrapper);
};

#endif //GENETIC_ACTOR_SVM_MUTATION_H

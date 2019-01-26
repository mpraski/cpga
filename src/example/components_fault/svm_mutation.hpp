//
// Created by marcin on 11/01/19.
//

#ifndef GENETIC_ACTOR_SVM_MUTATION_H
#define GENETIC_ACTOR_SVM_MUTATION_H

#include <core.hpp>
#include "components_fault_defs.hpp"

/**
 * @brief Mutation genetic operator for the component fault prediction problem.
 * @details This class performs mutation on rbf_params individuals. The new C and gamma
 * values are chosen as follows: add random double value in range -r..r to exisiting parameter value,
 * then make sure the result is within acceptable parameter range. The r is defined by constants::MUTATION_RANGE_C and
 * constants::MUTATION_RANGE_GAMMA for C and gamma respectively, whereas the acceptable ranges are the same ones used
 * by svm_initialization (constants::RANGE_C and constants::RANGE_GAMMA).
 */
class svm_mutation : public base_operator {
 private:
  std::default_random_engine generator;
  double min_c;
  double max_c;
  double min_gamma;
  double max_gamma;
  std::uniform_real_distribution<double> dist_mutate;
  std::uniform_real_distribution<double> dist_mutate_c;
  std::uniform_real_distribution<double> dist_mutate_gamma;

  static inline auto make_range(double a) {
    return std::uniform_real_distribution<double>{-a, a};
  }

  inline auto from_range(std::tuple<double, double> range) {
    return std::uniform_real_distribution<double>{std::get<0>(range), std::get<1>(range)};
  }
 public:
  svm_mutation() = default;
  svm_mutation(const shared_config &config, island_id island_no);

  void operator()(wrapper<rbf_params, double> &wrapper);
};

#endif //GENETIC_ACTOR_SVM_MUTATION_H

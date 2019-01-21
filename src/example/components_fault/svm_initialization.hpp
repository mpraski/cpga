//
// Created by marcin on 11/01/19.
//

#ifndef GENETIC_ACTOR_SVM_INITIALIZATION_H
#define GENETIC_ACTOR_SVM_INITIALIZATION_H

#include <random>
#include <core.hpp>
#include "components_fault_defs.hpp"

class svm_initialization : public base_operator {
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> dist_c;
  std::uniform_real_distribution<double> dist_gamma;

  inline auto from_range(std::tuple<double, double> range) {
    return std::uniform_real_distribution<double>{std::get<0>(range), std::get<1>(range)};
  }
 public:
  svm_initialization() = default;
  svm_initialization(const shared_config &config, island_id island_no);

  void operator()(inserter<rbf_params, double> it);
};

#endif //GENETIC_ACTOR_SVM_INITIALIZATION_H

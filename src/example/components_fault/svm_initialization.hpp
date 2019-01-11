//
// Created by marcin on 11/01/19.
//

#ifndef GENETIC_ACTOR_SVM_INITIALIZATION_H
#define GENETIC_ACTOR_SVM_INITIALIZATION_H

#include <random>
#include <core.hpp>
#include "components_fault_defs.hpp"

class svm_initialization : public base_operator<rbf_params, double> {
  INCLUDES(rbf_params, double);
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> dist_c;
  std::uniform_real_distribution<double> dist_gamma;
 public:
  svm_initialization(const shared_config &config, island_id island_no);

  void operator()(inserter it);
};

#endif //GENETIC_ACTOR_SVM_INITIALIZATION_H

//
// Created by marcinpraski on 10/01/19.
//

#ifndef GENETIC_ACTOR_SVM_FITNESS_EVALUATION_H
#define GENETIC_ACTOR_SVM_FITNESS_EVALUATION_H

#include <core.hpp>
#include "vendor/libsvm/svm.hpp"
#include "components_fault_defs.hpp"
#include "components_fault_defs.hpp"

class svm_fitness_evaluation : public base_operator {
 private:
  int n_rows;
  int n_cols;
  int n_folds;
  double *cv_result;
  svm_problem *problem;

  svm_parameter *create_parameter(const rbf_params &params) const;
 public:
  svm_fitness_evaluation(const shared_config& config, island_id island_no);
  ~svm_fitness_evaluation();

  double operator()(const rbf_params &ind) const;
};

#endif //GENETIC_ACTOR_SVM_FITNESS_EVALUATION_H

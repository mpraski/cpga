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
  svm_parameter parameter;
  svm_problem problem;

  constexpr static double eps = 1e-10;

  inline static bool non_zero(double d) {
    return d > eps || d < -eps;
  }

  svm_parameter create_parameter() const;
  svm_problem create_problem() const;
  void free_memory();
 public:
  svm_fitness_evaluation() = default;
  svm_fitness_evaluation(svm_fitness_evaluation &&other);
  svm_fitness_evaluation(const svm_fitness_evaluation &other) = delete;
  ~svm_fitness_evaluation();

  svm_fitness_evaluation(const shared_config &config, island_id island_no);

  svm_fitness_evaluation &operator=(svm_fitness_evaluation &&other);
  svm_fitness_evaluation &operator=(const svm_fitness_evaluation &other) = delete;
  double operator()(const rbf_params &ind);
};

#endif //GENETIC_ACTOR_SVM_FITNESS_EVALUATION_H

//
// Created by marcinpraski on 10/01/19.
//

#include <utilities/csv_reader.h>
#include "svm_fitness_evaluation.hpp"

svm_fitness_evaluation::svm_fitness_evaluation(const std::string &csv_file, int n_rows, int n_cols, int n_folds)
    : n_rows{n_rows},
      n_cols{n_cols},
      n_folds{n_folds},
      cv_result{
          new double[n_rows]} {
  auto parsed = csv_reader<double>::read(csv_file, n_rows, n_cols);
}

svm_parameter *svm_fitness_evaluation::create_parameter(const rbf_params &params) const {
  return new svm_parameter{
      C_SVC,        /* svm_type */
      RBF,          /* kernel_type */
      0,            /* degree */
      params.gamma, /* gamma */
      0,            /* coef0 */

      100,          /* cache_size */
      0.1,          /* eps */
      params.c,     /* C */
      0,            /* nr_weight */
      nullptr,      /* weight_label */
      nullptr,      /* weight */
      0,            /* nu */
      0,            /* p */
      1,            /* shrinking */
      0             /* probability */
  };
}

double svm_fitness_evaluation::operator()(const rbf_params &ind) const {
  auto *parameter = create_parameter(ind);
  auto *model = svm_train(problem, parameter);

  //...

  svm_free_and_destroy_model(&model);
  delete parameter;

  return 0;
}

svm_fitness_evaluation::~svm_fitness_evaluation() {
  delete[] cv_result;
  delete problem;
}
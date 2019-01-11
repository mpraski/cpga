//
// Created by marcinpraski on 10/01/19.
//

#include <utilities/csv_reader.h>
#include "svm_fitness_evaluation.hpp"

svm_fitness_evaluation::svm_fitness_evaluation(const shared_config &config, island_id island_no)
    : base_operator{config, island_no},
      n_rows{std::any_cast<int>(config->user_props.at(constants::N_ROWS))},
      n_cols{std::any_cast<int>(config->user_props.at(constants::N_COLS))},
      n_folds{std::any_cast<int>(config->user_props.at(constants::N_FOLDS))},
      cv_result{new double[n_rows]} {
  const auto &csv_file = std::any_cast<const std::string &>(config->user_props.at(constants::CSV_FILE));
  auto parsed = csv_reader<double>::read(csv_file, n_rows, n_cols);
  auto *labels = new double[n_rows];
  auto **nodes = new svm_node *[n_rows];

  auto adder = [&](auto i) {
    return [&, i](auto acc, auto j) {
      return parsed[i][j] ? ++acc : acc;
    };
  };

  for (int i = 0; i < n_rows; ++i) {
    auto c{0};
    auto sparse_size{std::accumulate(std::begin(parsed[i]), std::end(parsed[i]), 1, adder(i))};
    labels[i] = parsed[i][0];
    nodes[i] = new svm_node[sparse_size];
    for (int j = 0; j < n_cols; ++j) {
      if (!parsed[i][j]) continue;
      nodes[i][c].index = j + 1;
      nodes[i][c].value = parsed[i][j];
      ++c;
    }
    nodes[i][c].index = -1;
  }

  problem = new svm_problem{n_rows, labels, nodes};
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

double svm_fitness_evaluation::operator()(const rbf_params &params) const {
  auto *parameter = create_parameter(params);
  auto *model = svm_train(problem, parameter);

  svm_cross_validation(problem, parameter, n_folds, cv_result);

  auto tp{0}, fp{0}, fn{0};
  for (int i = 0; i < n_rows; ++i) {
    if (problem->y[i]) {
      if (cv_result[i]) {
        ++tp;
      } else {
        ++fn;
      }
    } else if (cv_result[i]) {
      ++fp;
    }
  }

  svm_free_and_destroy_model(&model);
  svm_destroy_param(parameter);
  delete parameter;

  auto precision{static_cast<double>(tp) / (tp + fp)};
  auto recall{static_cast<double>(tp) / (tp + fn)};
  return (2 * precision * recall) / (precision + recall);
}

svm_fitness_evaluation::~svm_fitness_evaluation() {
  delete[] cv_result;
  delete problem->y;
  for (int i = 0; i < n_rows; ++i) {
    delete problem->x[i];
  }
  delete problem;
}
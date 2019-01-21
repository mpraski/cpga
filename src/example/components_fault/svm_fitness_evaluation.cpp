//
// Created by marcinpraski on 10/01/19.
//

#include <utilities/csv_reader.hpp>
#include "svm_fitness_evaluation.hpp"

void print_null(const char *) {}

svm_fitness_evaluation::svm_fitness_evaluation(const shared_config &config, island_id island_no)
    : base_operator{config, island_no},
      n_rows{std::any_cast<int>(config->user_props.at(constants::N_ROWS))},
      n_cols{std::any_cast<int>(config->user_props.at(constants::N_COLS))},
      n_folds{std::any_cast<int>(config->user_props.at(constants::N_FOLDS))},
      cv_result{new double[n_rows]},
      parameter{create_parameter()},
      problem{create_problem()} {
  svm_set_print_string_function(&print_null);
}

svm_fitness_evaluation::svm_fitness_evaluation(svm_fitness_evaluation &&other) {
  if (this != &other) {
    if (cv_result) free_memory();

    n_rows = other.n_rows;
    n_cols = other.n_cols;
    n_folds = other.n_folds;
    cv_result = other.cv_result;
    parameter = other.parameter;
    problem = other.problem;

    other.cv_result = nullptr;
  }
}

svm_parameter svm_fitness_evaluation::create_parameter() const {
  return svm_parameter{
      C_SVC,        /* svm_type */
      RBF,          /* kernel_type */
      3,            /* degree */
      1,            /* gamma */
      0,            /* coef0 */

      100,          /* cache_size */
      1e-1,         /* eps */
      1,            /* C */
      0,            /* nr_weight */
      nullptr,      /* weight_label */
      nullptr,      /* weight */
      0.5,          /* nu */
      0.1,          /* p */
      1,            /* shrinking */
      0             /* probability */
  };
}

svm_problem svm_fitness_evaluation::create_problem() const {
  auto csv_file = std::any_cast<std::string>(config->user_props.at(constants::CSV_FILE));
  auto parsed = csv_reader::read_double(csv_file, n_rows, n_cols);
  auto *labels = new double[n_rows];
  auto **nodes = new svm_node *[n_rows];

  auto adder = [](auto acc, auto d) {
    return non_zero(d) ? ++acc : acc;
  };

  for (int i = 0; i < n_rows; ++i) {
    auto c{0};
    auto sparse_size{
        std::accumulate(
            std::next(std::begin(parsed[i])),
            std::end(parsed[i]),
            1,
            adder
        )
    };
    labels[i] = parsed[i][0];
    nodes[i] = new svm_node[sparse_size];

    for (int j = 1; j < n_cols; ++j) {
      if (!non_zero(parsed[i][j])) continue;
      nodes[i][c].index = j;
      nodes[i][c].value = parsed[i][j];
      ++c;
    }

    nodes[i][c].index = -1;
  }

  return svm_problem{n_rows, labels, nodes};
}

double svm_fitness_evaluation::operator()(const rbf_params &params) {
  parameter.C = params.c;
  parameter.gamma = params.gamma;

  if (auto *error{svm_check_parameter(&problem, &parameter)}; error) {
    return 0;
  }

  svm_cross_validation(&problem, &parameter, n_folds, cv_result);

  auto tp{0}, fp{0}, fn{0};
  for (int i = 0; i < problem.l; ++i) {
    if (non_zero(problem.y[i])) {
      if (non_zero(cv_result[i])) {
        ++tp;
      } else {
        ++fn;
      }
    } else if (non_zero(cv_result[i])) {
      ++fp;
    }
  }

  auto precision{static_cast<double>(tp) / (tp + fp)};
  auto recall{static_cast<double>(tp) / (tp + fn)};
  return (2 * precision * recall) / (precision + recall);
}

void svm_fitness_evaluation::free_memory() {
  delete[] cv_result;
  delete[] problem.y;
  for (int i = 0; i < problem.l; ++i) {
    delete[] problem.x[i];
  }
  delete[] problem.x;
}

svm_fitness_evaluation &svm_fitness_evaluation::operator=(svm_fitness_evaluation &&other) {
  if (this != &other) {
    if (cv_result) free_memory();

    n_rows = other.n_rows;
    n_cols = other.n_cols;
    n_folds = other.n_folds;
    cv_result = other.cv_result;
    parameter = other.parameter;
    problem = other.problem;

    other.cv_result = nullptr;
  }
  return *this;
}

svm_fitness_evaluation::~svm_fitness_evaluation() {
  if (cv_result) free_memory();
}
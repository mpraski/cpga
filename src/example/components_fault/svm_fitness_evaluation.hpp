//
// Created by marcinpraski on 10/01/19.
//

#ifndef GENETIC_ACTOR_SVM_FITNESS_EVALUATION_H
#define GENETIC_ACTOR_SVM_FITNESS_EVALUATION_H

#include <core.hpp>
#include <threads.h>
#include "vendor/libsvm/svm.hpp"
#include "components_fault_defs.hpp"
#include "components_fault_defs.hpp"

/**
 * @brief Fitness evaluation operator for the component fault prediction problem.
 * @details This class performs fitness evaluation of the radial basis function (RBF) kernel
 * parameters used to train a SVM predicting whether a given software component is faulty or not.
 * Fitness value if computed by first applying a 5-fold cross-validation on the training set and
 * taking the F-measure of the resulting test. The dataset is accessed by reading appropriate number of
 * rows from a specifically encoded CSV files. For this reason this class expects following user parameters to be present:
 * @li constants::CSV_FILE: The path to CSV file
 * @li constants::N_ROWS: The number of rows to be read from the CSV file
 * @li constants::N_COLS: The number of columns to be read from the CSV file
 * @li constants::N_FOLDS: THe number of folds for cross-validation
 * @line
 * The CSV files can only include numerical values (that can be parsed using std::stod), and the first column has
 * to contain the class assigned to this data point (1 or 0). Remaining rows form the attribute vector.
 * No header is expected.
 * @note This class can only be move constructed or assigned (to facilitate reasoning about memory dynamically allocated
 * for the cross validation result and other LibSVM data.
 */
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
  svm_fitness_evaluation(svm_fitness_evaluation &&other) noexcept;
  svm_fitness_evaluation(const svm_fitness_evaluation &other) = delete;
  ~svm_fitness_evaluation();

  svm_fitness_evaluation(const shared_config &config, island_id island_no);

  svm_fitness_evaluation &operator=(svm_fitness_evaluation &&other) noexcept;
  svm_fitness_evaluation &operator=(const svm_fitness_evaluation &other) = delete;
  double operator()(const rbf_params &ind);
};

#endif //GENETIC_ACTOR_SVM_FITNESS_EVALUATION_H

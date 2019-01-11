//
// Created by marcin on 11/01/19.
//

#ifndef GENETIC_ACTOR_SVM_MUTATION_H
#define GENETIC_ACTOR_SVM_MUTATION_H

#include <core.hpp>
#include "components_fault_defs.hpp"

class svm_mutation : public base_operator<rbf_params, double> {
  INCLUDES(rbf_params, double);
 public:
  svm_mutation(const shared_config &config, island_id island_no);

  void operator()(wrapper &wrapper) const noexcept;
};

#endif //GENETIC_ACTOR_SVM_MUTATION_H

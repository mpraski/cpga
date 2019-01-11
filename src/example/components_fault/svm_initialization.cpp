//
// Created by marcin on 11/01/19.
//

#include "svm_initialization.hpp"

svm_initialization::svm_initialization(const shared_config &config, island_id island_no)
    : base_operator{config, island_no},
      generator{get_seed(config->system_props.initialization_seed)},
      dist_c{0.01, 32000.0},
      dist_gamma{1.0E-6, 8.0} {

}

void svm_initialization::operator()(inserter it) {
  auto &props = config->system_props;

  for (size_t i = 0; i < props.population_size; ++i) {
    it = {rbf_params{dist_c(generator), dist_gamma(generator)}, 0};
  }
}
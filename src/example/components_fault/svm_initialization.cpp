//
// Created by marcin on 11/01/19.
//

#include "svm_initialization.hpp"

svm_initialization::svm_initialization(const shared_config &config, island_id island_no)
    : base_operator{config, island_no},
      generator{get_seed(config->system_props.initialization_seed)},
      dist_c{from_range(std::any_cast<std::pair<double, double>>(config->user_props.at(constants::RANGE_C)))},
      dist_gamma{from_range(std::any_cast<std::pair<double, double>>(config->user_props.at(constants::RANGE_GAMMA)))} {

}

/**
 * @brief Creates props.population_size new individuals and inserts them to the population.
 * @param it the back_insert_iterator for the population
 */
void svm_initialization::operator()(inserter<rbf_params, double> it) {
  auto &props = config->system_props;

  for (size_t i = 0; i < props.population_size; ++i) {
    it = {rbf_params{dist_c(generator), dist_gamma(generator)}, 0};
  }
}
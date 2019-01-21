//
// Created by marcin on 11/01/19.
//

#include "svm_mutation.hpp"

svm_mutation::svm_mutation(const shared_config &config, island_id island_no)
    : base_operator{config, island_no},
      generator{config->system_props.mutation_seed},
      dist_mutate{0.0, 1.0},
      dist_mutate_c{make_range(std::any_cast<double>(config->user_props.at(constants::MUTATION_RANGE_C)))},
      dist_mutate_gamma{make_range(std::any_cast<double>(config->user_props.at(constants::MUTATION_RANGE_GAMMA)))} {
}

void svm_mutation::operator()(wrapper<rbf_params, double> &wrapper) {
  if (dist_mutate(generator) < config->system_props.mutation_probability) {
    wrapper.first.c = std::abs(wrapper.first.c + dist_mutate_c(generator));
    wrapper.first.gamma = std::abs(wrapper.first.gamma + dist_mutate_gamma(generator));
  }
}
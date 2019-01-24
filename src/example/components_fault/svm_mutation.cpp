//
// Created by marcin on 11/01/19.
//

#include "svm_mutation.hpp"

svm_mutation::svm_mutation(const shared_config &config, island_id island_no)
    : base_operator{config, island_no},
      generator{get_seed(config->system_props.mutation_seed)},
      min_c{std::get<0>(std::any_cast<std::pair<double, double>>(config->user_props.at(constants::RANGE_C)))},
      max_c{std::get<1>(std::any_cast<std::pair<double, double>>(config->user_props.at(constants::RANGE_C)))},
      min_gamma{std::get<0>(std::any_cast<std::pair<double, double>>(config->user_props.at(constants::RANGE_GAMMA)))},
      max_gamma{std::get<1>(std::any_cast<std::pair<double, double>>(config->user_props.at(constants::RANGE_GAMMA)))},
      dist_mutate{0.0, 1.0},
      dist_mutate_c{make_range(std::any_cast<double>(config->user_props.at(constants::MUTATION_RANGE_C)))},
      dist_mutate_gamma{make_range(std::any_cast<double>(config->user_props.at(constants::MUTATION_RANGE_GAMMA)))} {
}

void svm_mutation::operator()(wrapper<rbf_params, double> &wrapper) {
  if (dist_mutate(generator) < config->system_props.mutation_probability) {
    wrapper.first.c = std::clamp(
        wrapper.first.c + dist_mutate_c(generator),
        min_c,
        max_c
    );
    wrapper.first.gamma = std::clamp(
        wrapper.first.gamma + dist_mutate_gamma(generator),
        min_gamma,
        max_gamma
    );
  }
}
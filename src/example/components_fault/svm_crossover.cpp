//
// Created by marcin on 11/01/19.
//

#include "svm_crossover.hpp"

svm_crossover::svm_crossover(const shared_config &config, island_id island_no)
    : base_operator{config, island_no},
      generator{get_seed(config->system_props.crossover_seed)},
      distribution{config->system_props.crossover_probability},
      coin_toss{std::bind(distribution, generator)} {

}

void svm_crossover::operator()(inserter<rbf_params, double> it,
                               const wrapper_pair<rbf_params, double> &couple) {
  auto &parent1 = couple.first.first;
  auto &parent2 = couple.second.first;

  auto produce = [&] {
    auto toss = coin_toss();
    return rbf_params{
        toss ? parent1.c : parent2.c,
        toss ? parent2.gamma : parent1.gamma
    };
  };

  it = {produce(), 0};
  it = {produce(), 0};
}
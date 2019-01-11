//
// Created by marcin on 11/01/19.
//

#include "svm_crossover.hpp"

svm_crossover::svm_crossover(const shared_config &config, island_id island_no)
    : base_operator{config, island_no},
      generator{get_seed(config->system_props.crossover_seed)},
      distribution{0.5},
      coin_toss{std::bind(distribution, generator)} {

}

void svm_crossover::operator()(inserter it, const wrapper_pair &couple) {
  auto &parent1 = couple.first.first;
  auto &parent2 = couple.second.first;

  auto produce = [&] {
    return rbf_params{
        coin_toss() ? parent1.c : parent2.c,
        coin_toss() ? parent1.gamma : parent2.gamma
    };
  };

  it = {produce(), 0};
  it = {produce(), 0};
}
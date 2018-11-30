#pragma once

#include "../core.hpp"

#include <random>
#include <vector>

template<typename sequence_individual, typename fitness_value>
class sequence_individual_crossover : public base_state {
  using individual = std::vector<sequence_individual>;
 private:
  std::default_random_engine generator;
  std::uniform_int_distribution<std::size_t> distribution;
  std::function<std::size_t()> random;
 public:
  sequence_individual_crossover() = default;

  sequence_individual_crossover(const shared_config& config)
      : base_state { config },
        generator { config->system_props.crossover_seed },
        distribution { 0, config->system_props.individual_size },
        random { std::bind(distribution, generator) } {
  }

  void operator()(
      individual_collection<individual, fitness_value>& children,
      const individual_wrapper_pair<individual, fitness_value>& parents) const {
    auto ind_size = config->system_props.individual_size;

    individual child1(ind_size);
    individual child2(ind_size);

    auto rand = random();

    for (std::size_t i = 0; i < ind_size; ++i) {
      if (i <= rand) {
        child1[i] = parents.first.first[i];
        child2[i] = parents.second.first[i];
      } else {
        child1[i] = parents.second.first[i];
        child2[i] = parents.first.first[i];
      }
    }

    children.emplace_back(std::move(child1), fitness_value { });
    children.emplace_back(std::move(child2), fitness_value { });
  }
};

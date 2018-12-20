#pragma once

#include "../core.hpp"
#include "sequence_individual_factory.hpp"

#include <random>
#include <vector>

template<typename individual, typename fitness_value, typename factory = sequence_individual_vector_factory<typename individual::value_type>>
class sequence_individual_crossover : public base_operator {
 private:
  factory create;
  std::default_random_engine generator;
  std::uniform_int_distribution<size_t> distribution;
  std::function<size_t()> random_f;
 public:
  sequence_individual_crossover() = default;
  sequence_individual_crossover(const shared_config &config,
                                island_id island_no)
      : base_operator{config, island_no},
        create{},
        generator{get_seed(config->system_props.crossover_seed)},
        distribution{0, config->system_props.individual_size},
        random_f{std::bind(distribution, generator)} {
  }

  void operator()(
      individual_collection<individual, fitness_value> &children,
      const individual_wrapper_pair<individual, fitness_value> &parents) const {
    auto ind_size = config->system_props.individual_size;

    individual child1 = create(ind_size);
    individual child2 = create(ind_size);
    auto it1 = std::begin(child1);
    auto it2 = std::begin(child2);

    auto rand = random_f();

    for (size_t i = 0; i < ind_size; ++i) {
      if (i <= rand) {
        *(it1++) = parents.first.first[i];
        *(it2++) = parents.second.first[i];
      } else {
        *(it1++) = parents.second.first[i];
        *(it2++) = parents.first.first[i];
      }
    }

    children.emplace_back(std::move(child1), fitness_value{});
    children.emplace_back(std::move(child2), fitness_value{});
  }
};

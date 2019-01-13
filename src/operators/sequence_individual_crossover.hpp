#pragma once

#include <random>
#include <vector>
#include <core.hpp>

template<typename constituent, typename fitness_value, typename individual = std::vector<constituent>>
class sequence_individual_crossover : public base_operator {
 private:
  std::default_random_engine generator;
  std::uniform_int_distribution<size_t> distribution;
  std::function<size_t()> random_f;

  inline individual create() const {
    if constexpr (is_size_constructible<individual>()) {
      return individual(config->system_props.individual_size);
    } else {
      static_assert(std::is_default_constructible<individual>::value,
                    "create() requires a default constructible container");
      return individual();
    }
  }
 public:
  sequence_individual_crossover() = default;
  sequence_individual_crossover(const shared_config &config,
                                island_id island_no)
      : base_operator{config, island_no},
        generator{get_seed(config->system_props.crossover_seed)},
        distribution{0, config->system_props.individual_size},
        random_f{std::bind(distribution, generator)} {
  }

  void operator()(inserter<individual, fitness_value> it,
                  const wrapper_pair<individual, fitness_value> &couple) const {
    auto ind_size = config->system_props.individual_size;

    auto child1 = create();
    auto child2 = create();
    auto it1 = std::begin(child1);
    auto it2 = std::begin(child2);
    auto itp1 = std::begin(couple.first.first);
    auto itp2 = std::begin(couple.second.first);
    auto rand = random_f();

    for (size_t i = 0; i < ind_size; ++i) {
      if (i <= rand) {
        *(it1++) = *(itp1++);
        *(it2++) = *(itp2++);
      } else {
        *(it1++) = *(itp2++);
        *(it2++) = *(itp1++);
      }
    }

    it = {std::move(child1), fitness_value{}};
    it = {std::move(child2), fitness_value{}};
  }
};

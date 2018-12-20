#pragma once

#include "../core.hpp"

#include <random>

template<typename individual, typename fitness_value>
class roulette_wheel_parent_selection : base_operator {
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution;
  std::function<double()> random_one;

  inline size_t spin(
      const fitness_value &total_fitness,
      const individual_collection<individual, fitness_value> &population) const
  noexcept {
    auto rand_fitness{random_one() * total_fitness};
    size_t start{0};
    while (rand_fitness > 0) {
      rand_fitness -= population[start++].second;
    }
    return start - 1;
  }
 public:
  roulette_wheel_parent_selection() = default;
  roulette_wheel_parent_selection(const shared_config &config,
                                  island_id island_no)
      : base_operator{config, island_no},
        generator{get_seed(config->system_props.parent_selection_seed)},
        distribution{0.0, 1.0},
        random_one{std::bind(distribution, generator)} {

  }

  void operator()(individual_collection<individual, fitness_value> &population,
                  parent_collection<individual, fitness_value> &couples) const {
    auto couples_num = population.size() / 2;

    fitness_value total{};
    for (const auto &[ind, value] : population) {
      total += value;
    }

    for (size_t i = 0; i < couples_num; ++i) {
      auto first = spin(total, population);
      auto second = spin(total, population);

      if (second == first) {
        second = (second + 1) % population.size();
      }

      couples.emplace_back(population[first], population[second]);
    }
  }

};

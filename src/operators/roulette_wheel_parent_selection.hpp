#pragma once

#include "../core.hpp"

#include <random>

template<typename individual, typename fitness_value>
class roulette_wheel_parent_selection : base_state {
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution;
  std::function<double()> random;

  inline std::size_t spin(
      fitness_value total_fitness,
      const individual_collection<individual, fitness_value>& population) const
          noexcept {
    auto rand_fitness = random() * total_fitness;
    auto start = 0;
    while (rand_fitness > 0) {
      rand_fitness -= population[start++].second;
    }
    return start - 1;
  }
 public:
  roulette_wheel_parent_selection() = default;

  roulette_wheel_parent_selection(const shared_config& config)
      : base_state { config },
        generator { config->system_props.parent_selection_seed },
        distribution { 0.0, 1.0 },
        random { std::bind(distribution, generator) } {

  }

  void operator()(individual_collection<individual, fitness_value>& population,
                  parent_collection<individual, fitness_value>& couples) const {
    auto couples_num = population.size() / 2;

    fitness_value total { };
    for (const auto& wrapper : population) {
      total += wrapper.second;
    }

    std::size_t first, second;
    for (std::size_t i = 0; i < couples_num; ++i) {
      first = spin(total, population);
      second = spin(total, population);

      if (second == first) {
        second = (second + 1) % population.size();
      }

      couples.emplace_back(population[first], population[second]);
    }
  }

};

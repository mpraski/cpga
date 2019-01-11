#pragma once

#include <random>
#include <core.hpp>

template<typename individual, typename fitness_value>
class roulette_wheel_survival_selection : public base_operator<individual, fitness_value> {
  INCLUDES(individual, fitness_value);
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution;
  std::function<double()> random_one;

  inline size_t spin(const fitness_value &total_fitness, const population &population) const noexcept {
    auto rand_fitness{random_one() * total_fitness};
    size_t start{0};
    while (rand_fitness > 0) {
      rand_fitness -= population[start++].second;
    }
    return start - 1;
  }
 public:
  roulette_wheel_survival_selection() = default;
  roulette_wheel_survival_selection(const shared_config &config,
                                    island_id island_no)
      : base_operator<individual, fitness_value>{config, island_no},
        generator{get_seed(config->system_props.survival_selection_seed)},
        distribution{0.0, 1.0},
        random_one{std::bind(distribution, generator)} {

  }

  void operator()(population &parents, population &offspring) const {
    auto survivors_num = parents.size();

    parents.reserve(parents.size() + offspring.size());
    parents.insert(parents.end(),
                   std::make_move_iterator(offspring.begin()),
                   std::make_move_iterator(offspring.end()));
    offspring.clear();

    auto total = std::accumulate(std::begin(parents),
                                 std::end(parents),
                                 fitness_value{},
                                 [](auto acc, const auto &m) { return acc + m.second; });

    for (size_t i = 0; i < survivors_num; ++i) {
      auto survivor = spin(total, parents);

      offspring.emplace_back(std::move(parents[survivor]));
      offspring.erase(offspring.begin() + survivor);
    }
  }
};

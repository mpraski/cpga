#ifndef GENETIC_ACTOR_ROULETTE_WHEEL_PARENT_SELECTION_H
#define GENETIC_ACTOR_ROULETTE_WHEEL_PARENT_SELECTION_H

#include <random>
#include "../core.hpp"

namespace cpga {
namespace operators {
/**
 * @brief Genetic operator performing parent selection for crossover using roulette-wheel method.
 * @details This class perform parent selection bt applying the
 * <a href="https://en.wikipedia.org/wiki/Fitness_proportionate_selection">fitness proportionate selection</a>.
 * @tparam individual
 * @tparam fitness_value
 */
template<typename individual, typename fitness_value>
class roulette_wheel_parent_selection : public base_operator {
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution;
  std::function<double()> random_one;

  inline size_t spin(
      const fitness_value &total_fitness,
      const population<individual, fitness_value> &population) const
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

  /**
   * @brief Fills couples with selected individidual pairs.
   * @param population the common population
   * @param couples the collection of resulting couples (a vector of wrapper pairs)
   */
  void operator()(population<individual, fitness_value> &population,
                  couples<individual, fitness_value> &couples) const {
    auto couples_num = population.size() / 2;

    auto total = std::accumulate(std::begin(population),
                                 std::end(population),
                                 fitness_value{},
                                 [](auto acc, const auto &m) { return acc + m.second; });

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
}
}

#endif //GENETIC_ACTOR_ROULETTE_WHEEL_PARENT_SELECTION_H
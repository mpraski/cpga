#ifndef GENETIC_ACTOR_AVERAGE_GLOBAL_TERMINATION_CHECK_H
#define GENETIC_ACTOR_AVERAGE_GLOBAL_TERMINATION_CHECK_H

#include <core.hpp>

namespace cpga {
namespace operators {
/**
 * @brief Genetic operator determining whether a stopping condition has been reached.
 * @details This class computes the average fitness value of a population and compares
 * it with a specified average. If it is greater for n consecutive times then the stopping
 * condition has been reached. The minimum average is specified by constants::STABLE_REQUIRED_KEY,
 * and the number of consecutive successful checks by constants::MINIMUM_AVERAGE_KEY.
 * @tparam individual
 * @tparam fitness_value
 */
template<typename individual, typename fitness_value>
class average_fitness_global_termination_check : public base_operator {
 private:
  size_t stable_so_far;
  size_t stable_required;
  fitness_value minimum_average;
 public:
  average_fitness_global_termination_check() = default;
  average_fitness_global_termination_check(const shared_config &config,
                                           island_id island_no)
      : base_operator{config, island_no},
        stable_so_far{0},
        stable_required{std::any_cast<size_t>(
            config->user_props.at(constants::STABLE_REQUIRED_KEY))},
        minimum_average{std::any_cast<fitness_value>(
            config->user_props.at(constants::MINIMUM_AVERAGE_KEY))} {
  }

  /**
   * @bried perform actual check on a population.
   * @param population the common population
   * @return Whether the stopping condition has been reached.
   */
  bool operator()(const population<individual, fitness_value> &population) noexcept {
    if (population.empty()) {
      return true;
    }

    fitness_value total = std::accumulate(std::begin(population),
                                          std::end(population),
                                          fitness_value{},
                                          [](auto acc, const auto &m) { return acc + m.second; }) / population.size();

    if (total >= minimum_average) {
      return ++stable_so_far == stable_required;
    }

    stable_so_far = 0;
    return false;
  }
};
}
}

#endif //GENETIC_ACTOR_AVERAGE_GLOBAL_TERMINATION_CHECK_H
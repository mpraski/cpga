#include "../core.hpp"

template<typename individual, typename fitness_value>
class average_fitness_global_termination_check : public base_state {
 private:
  std::size_t stable_so_far;
  std::size_t stable_required;
  fitness_value minimun_average;
 public:
  average_fitness_global_termination_check() = default;

  average_fitness_global_termination_check(const shared_config& config)
      : base_state { config },
        stable_so_far { 0 },
        stable_required { std::any_cast<std::size_t>(
            config->user_props.at(constants::STABLE_REQUIRED_KEY)) },
        minimun_average { std::any_cast<fitness_value>(
            config->user_props.at(constants::MINIMUM_AVERAGE_KEY)) } {
  }

  bool operator()(
      const individual_collection<individual, fitness_value>& population)
          noexcept {
    if (population.empty()) {
      return true;
    }

    fitness_value total { };
    for (const auto& member : population) {
      total += member.second;
    }

    total = total / population.size();

    if (total >= minimun_average) {
      return ++stable_so_far == stable_required;
    }

    stable_so_far = 0;
    return false;
  }
};

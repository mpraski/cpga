#pragma once

#include "../core.hpp"

#include <algorithm>
#include <vector>

template<typename individual, typename fitness_value>
class best_individual_elitism : public base_state {
  using Comparator = std::function<bool(individual_wrapper<individual, fitness_value>, individual_wrapper<individual, fitness_value>)>;
 private:
  Comparator compFunctor = [](const auto& w1, const auto& w2)
  {
    return w1.second > w2.second;
  };
 public:
  best_individual_elitism() = default;

  best_individual_elitism(const shared_config& config)
      : base_state { config } {
  }

  void operator()(
      individual_collection<individual, fitness_value>& population,
      individual_collection<individual, fitness_value>& elitists) const {
    std::sort(population.begin(), population.end(), compFunctor);

    elitists.insert(
        elitists.end(),
        std::make_move_iterator(population.begin()),
        std::make_move_iterator(
            population.begin() + config->system_props.elitists_number));

    population.erase(population.begin(),
                     population.begin() + config->system_props.elitists_number);
  }
};

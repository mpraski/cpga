#pragma once

#include <algorithm>
#include <vector>
#include <core.hpp>

template<typename individual, typename fitness_value>
class best_individual_elitism : public base_operator {
  using Comparator = std::function<bool(individual_wrapper<individual, fitness_value>,
                                        individual_wrapper<individual, fitness_value>)>;
 private:
  Comparator compFunctor = [](const auto &w1, const auto &w2) {
    return w1.second > w2.second;
  };
 public:
  using base_operator::base_operator;

  void operator()(
      individual_collection<individual, fitness_value> &population,
      individual_collection<individual, fitness_value> &elitists) const {
    std::sort(population.begin(), population.end(), compFunctor);

    auto end = population.begin() + config->system_props.elitists_number;

    elitists.insert(
        elitists.end(),
        std::make_move_iterator(population.begin()),
        std::make_move_iterator(end));

    population.erase(population.begin(), end);
  }
};

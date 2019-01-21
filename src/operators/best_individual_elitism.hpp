#pragma once

#include <algorithm>
#include <vector>
#include <core.hpp>
#include <utilities/population_sorter.hpp>

template<typename individual, typename fitness_value>
class best_individual_elitism : public base_operator {
 public:
  using base_operator::base_operator;

  void operator()(population<individual, fitness_value> &main,
                  population<individual, fitness_value> &elitists) const {
    population_sorter<individual, fitness_value>::sort(main);

    auto end = main.begin() + config->system_props.elitists_number;

    elitists.insert(
        elitists.end(),
        std::make_move_iterator(main.begin()),
        std::make_move_iterator(end));

    main.erase(main.begin(), end);
  }
};

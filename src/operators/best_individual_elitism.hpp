#pragma once

#include <algorithm>
#include <vector>
#include <core.hpp>

template<typename individual, typename fitness_value>
class best_individual_elitism : public base_operator<individual, fitness_value> {
  INCLUDES(individual, fitness_value);
 private:
  constexpr static auto compFunctor = [](const wrapper &w1, const wrapper &w2) {
    return w1.second > w2.second;
  };
 public:
  using base_operator<individual, fitness_value>::base_operator;

  void operator()(population &main, population &elitists) const {
    std::sort(main.begin(), main.end(), compFunctor);

    auto end = main.begin() + config->system_props.elitists_number;

    elitists.insert(
        elitists.end(),
        std::make_move_iterator(main.begin()),
        std::make_move_iterator(end));

    main.erase(main.begin(), end);
  }
};

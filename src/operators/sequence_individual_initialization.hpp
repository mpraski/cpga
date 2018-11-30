#pragma once

#include "../core.hpp"

#include <random>
#include <vector>

template<typename sequence_individual, typename fitness_value>
class sequence_individual_initialization : public base_state {
  using individual = std::vector<sequence_individual>;
  using distribution = std::uniform_int_distribution<std::size_t>;
 private:
  std::default_random_engine generator;
  std::vector<sequence_individual> possible_values;
 public:
  sequence_individual_initialization() = default;

  sequence_individual_initialization(const shared_config& configuration)
      : base_state { configuration },
        generator { configuration->system_props.initialization_seed },
        possible_values { std::any_cast<std::vector<sequence_individual>>(
            config->user_props.at(constants::POSSIBLE_VALUES_KEY)) } {
    if (possible_values.size() > config->system_props.individual_size) {
      throw std::runtime_error("Less possible values than individual size");
    }
  }

  void operator()(
      individual_collection<individual, fitness_value>& individuals) {
    auto& props = config->system_props;
    auto values = possible_values;

    for (std::size_t i = 0; i < props.population_size; ++i) {
      individual ind(props.individual_size);

      for (std::size_t j = 0; j < props.individual_size; ++j) {
        auto r = distribution { 0, values.size() - 1 }(generator);
        ind[j] = values[r];

        if (props.can_repeat_individual_elements) {
          values.erase(values.begin() + r);
        }
      }

      individuals.emplace_back(std::move(ind), fitness_value { });
    }
  }
};

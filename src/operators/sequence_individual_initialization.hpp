#pragma once

#include "../core.hpp"
#include "sequence_individual_factory.hpp"

#include <random>
#include <vector>

template<typename individual, typename fitness_value, typename factory = sequence_individual_vector_factory<typename individual::value_type>>
class sequence_individual_initialization : public base_operator {
  using distribution = std::uniform_int_distribution<size_t>;
  using value = typename individual::value_type;
 private:
  factory create;
  std::default_random_engine generator;
  std::vector<value> possible_values;
 public:
  sequence_individual_initialization() = default;
  sequence_individual_initialization(const shared_config &config,
                                     island_id island_no)
      : base_operator{config, island_no},
        create{},
        generator{get_seed(config->system_props.initialization_seed)},
        possible_values{std::any_cast<std::vector<value>>(
            config->user_props.at(constants::POSSIBLE_VALUES_KEY))} {
    if (!config->system_props.can_repeat_individual_elements
        && possible_values.size() < config->system_props.individual_size) {
      throw std::runtime_error("Less possible values than individual size");
    }
  }

  void operator()(
      individual_collection<individual, fitness_value> &individuals) {
    auto &props = config->system_props;
    auto values = possible_values;

    for (size_t i = 0; i < props.population_size; ++i) {
      individual ind = create(props.individual_size);
      auto it = std::begin(ind);

      for (size_t j = 0; j < props.individual_size; ++j) {
        auto r = distribution{0, values.size() - 1}(generator);
        *(it++) = values[r];

        if (!props.can_repeat_individual_elements) {
          erase_quick(values, values.begin() + r);
        }
      }

      individuals.emplace_back(std::move(ind), fitness_value{});
    }
  }
};

#pragma once

#include "../core.hpp"

#include <random>
#include <vector>

template<typename constituent, typename fitness_value, typename individual = std::vector<constituent>>
class sequence_individual_initialization : public base_operator {
  using distribution = std::uniform_int_distribution<size_t>;
 private:
  std::default_random_engine generator;
  std::vector<constituent> possible_values;

  inline individual create() const {
    if constexpr (is_size_constructible<individual>()) {
      return individual(config->system_props.individual_size);
    } else {
      static_assert(std::is_default_constructible<individual>::value,
                    "create() requires a default constructible container");
      return individual();
    }
  }
 public:
  sequence_individual_initialization() = default;
  sequence_individual_initialization(const shared_config &config,
                                     island_id island_no)
      : base_operator{config, island_no},
        generator{get_seed(config->system_props.initialization_seed)},
        possible_values{std::any_cast<std::vector<constituent>>(
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
      auto ind = create();
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

#pragma once

#include <random>
#include <vector>
#include <core.hpp>

template<typename constituent, typename fitness_value, typename individual = std::vector<constituent>>
class sequence_individual_initialization : public base_operator<individual, fitness_value> {
  INCLUDES(individual, fitness_value);
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
      : base_operator<individual, fitness_value>{config, island_no},
        generator{get_seed(config->system_props.initialization_seed)},
        possible_values{std::any_cast<std::vector<constituent>>(
            config->user_props.at(constants::POSSIBLE_VALUES_KEY))} {
    if (!config->system_props.can_repeat_individual_elements
        && possible_values.size() < config->system_props.individual_size) {
      throw std::runtime_error("Less possible values than individual size");
    }
  }

  void operator()(inserter it) {
    auto &props = config->system_props;
    auto values = possible_values;

    auto gen = [&] {
      auto r = std::uniform_int_distribution<size_t>{0, values.size() - 1}(generator);
      if (!props.can_repeat_individual_elements) {
        values.erase(values.begin() + r);
      }
      return values[r];
    };

    for (size_t i = 0; i < props.population_size; ++i) {
      auto ind = create();
      std::generate(std::begin(ind), std::end(ind), gen);
      it = {std::move(ind), fitness_value{}};
    }
  }
};

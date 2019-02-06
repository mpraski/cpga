#ifndef GENETIC_ACTOR_SEQUENCE_INDIVIDUAL_INITIALIZATION_H
#define GENETIC_ACTOR_SEQUENCE_INDIVIDUAL_INITIALIZATION_H

#include <random>
#include <vector>
#include "../core.hpp"

namespace cpga {
namespace operators {
/**
 * @brief Genetic operator performing initialisation of a population of 'sequence' individuals.
 * @details This class is meant to facilitate initializing a population of individuals who are
 * 'sequences' - standard library containers such as vector, list, set, or any custom type supporting
 * STL iterators. The user must define the size of the sequence (system_properties.individual_size), as
 * well as provide a vector of possible 'constituent' values (constants::POSSIBLE_VALUES_KEY) which will be
 * picked at random to create the sequence individual (e.g. a vector<bool>{true, false} to build a bitstring
 * individual.
 * @tparam constituent
 * @tparam fitness_value
 * @tparam individual
 */
template<typename constituent, typename fitness_value, typename individual = std::vector<constituent>>
class sequence_individual_initialization : public base_operator {
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

  /**
   * @brief Initialize the population by assigning newly created individuals to it.
   * @param it the back_insert_iterator for the population collection
   */
  void operator()(inserter<individual, fitness_value> it) {
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
}
}

#endif //GENETIC_ACTOR_SEQUENCE_INDIVIDUAL_INITIALIZATION_H
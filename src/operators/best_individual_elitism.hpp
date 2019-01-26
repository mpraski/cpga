#ifndef GENETIC_ACTOR_BEST_INDIVIDUAL_ELITISM_H
#define GENETIC_ACTOR_BEST_INDIVIDUAL_ELITISM_H

#include <algorithm>
#include <vector>
#include <core.hpp>
#include <utilities/population_sorter.hpp>

/**
 * @brief Genetic operator selecting and saving eletists from a common population.
 * @details This class performs elitism by first sorting the population by fitness
 * value in descending order, then moving first n individuals to the elitits population
 * and erasing them from the common one.
 * @tparam individual
 * @tparam fitness_value
 */
template<typename individual, typename fitness_value>
class best_individual_elitism : public base_operator {
 public:
  using base_operator::base_operator;

  /**
   * @brief Perform elitist selection and extraction.
   * @param main the commmon population from which the elitists are extracted.
   * @param elitists the population to which elitists are moved.
   */
  void operator()(population<individual, fitness_value> &main,
                  population<individual, fitness_value> &elitists) const {
    population_sorter<individual, fitness_value>::sort(main);

    auto end = std::next(main.begin(), config->system_props.elitists_number);

    elitists.insert(
        elitists.end(),
        std::make_move_iterator(main.begin()),
        std::make_move_iterator(end));

    main.erase(main.begin(), end);
  }
};

#endif //GENETIC_ACTOR_BEST_INDIVIDUAL_ELITISM_H
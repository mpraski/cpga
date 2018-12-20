#pragma once

#include "caf/all.hpp"
#include "../core.hpp"

using namespace caf;

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator = default_survival_selection_operator<
        individual, fitness_value>,
    typename elitism_operator = default_elitism_operator<individual,
                                                         fitness_value>,
    typename global_termination_check = default_global_termination_check<
        individual, fitness_value>>
class sequential_model_driver : public base_driver<individual, fitness_value> {
 private:
  void run_pga(const scoped_actor &self, const shared_config &config) {
    auto &props = config->system_props;

    fitness_evaluation_operator fitness_evaluation{config, island_0};
    initialization_operator initialization{config, island_0};
    crossover_operator crossover{config, island_0};
    mutation_operator mutation{config, island_0};
    parent_selection_operator parent_selection{config, island_0};
    survival_selection_operator survival_selection{config, island_0};
    elitism_operator elitism{config, island_0};
    global_termination_check termination_check{config, island_0};

    parent_collection<individual, fitness_value> parents;

    individual_collection<individual, fitness_value> population;
    individual_collection<individual, fitness_value> offspring;
    individual_collection<individual, fitness_value> elitists;

    parents.reserve(props.population_size / 2);
    population.reserve(props.population_size + props.elitists_number);
    offspring.reserve(props.population_size);
    elitists.reserve(props.population_size);

    if (props.is_generation_reporter_active) {
      self->send(config->generation_reporter, note_start::value, now());
    }

    initialization(population);

    for (size_t g = 0; g < props.generations_number; ++g) {
      for (auto &member : population) {
        member.second = fitness_evaluation(member.first);
      }

      if (props.is_elitism_active) {
        elitism(population, elitists);
      }

      // This will fill parents with individual_wrapper_pairs, each holding two copied individuals
      parent_selection(population, parents);

      // This will fill offspring with newly created individual_wrappers
      for (const auto &parent : parents) {
        crossover(offspring, parent);
      }

      // Clear parents for future use
      parents.clear();

      // This will apply mutation to each child in offspring
      for (auto &child : offspring) {
        mutation(child);
      }

      if (props.is_survival_selection_active) {
        for (auto &child : offspring) {
          child.second = fitness_evaluation(child.first);
        }

        survival_selection(population, offspring);
      }

      population.swap(offspring);
      offspring.clear();

      if (props.is_elitism_active) {
        population.insert(population.end(),
                          std::make_move_iterator(elitists.begin()),
                          std::make_move_iterator(elitists.end()));
        elitists.clear();
      }

      if (termination_check(population)) {
        break;
      }
    }

    if (props.is_generation_reporter_active) {
      auto &generation_reporter = config->generation_reporter;
      self->send(generation_reporter, note_end::value, now(),
                 actor_phase::total, props.generations_number, island_0);
    }

    if (props.is_individual_reporter_active) {
      auto &individual_reporter = config->individual_reporter;

      self->send(individual_reporter, report_population::value, population,
                 props.generations_number, island_0);
    }
  }

 public:
  using base_driver<individual, fitness_value>::base_driver;

  void perform(shared_config &config, actor_system &system, scoped_actor &self)
  override {
    run_pga(self, config);
  }
};

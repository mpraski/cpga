#pragma once

#include "caf/all.hpp"
#include "../core.hpp"

using namespace caf;

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator, typename survival_selection_operator,
    typename elitism_operator, typename global_temination_check>
class sequential_model_driver : private base_driver {
 private:
  void run_pga(const scoped_actor& self, const shared_config& config,
               fitness_evaluation_operator& fitness_evaluation,
               initialization_operator& initialization,
               crossover_operator& crossover, mutation_operator& mutation,
               parent_selection_operator& parent_selection,
               survival_selection_operator& survival_selection,
               elitism_operator& elitism,
               global_temination_check& termination_check) {
    auto& props = config->system_props;

    parent_collection<individual, fitness_value> parents;

    individual_collection<individual, fitness_value> population;
    individual_collection<individual, fitness_value> offspring;
    individual_collection<individual, fitness_value> elitists;

    parents.reserve(props.population_size / 2);
    population.reserve(props.population_size + props.elitists_number);
    offspring.reserve(props.population_size);
    elitists.reserve(props.population_size);

    if (props.is_actor_reporter_active) {
      self->send(config->actor_reporter, note_start::value, constants::now());
    }

    initialization(population);

    for (std::size_t g = 0; g < props.generations_number; ++g) {
      for (auto& member : population) {
        member.second = fitness_evaluation(member.first);
      }

      if (props.is_elitism_active) {
        elitism(population, elitists);
      }

      // This will fill parents with individual_wrapper_pairs, each holding two copied individuals
      parent_selection(population, parents);

      // This will fill offspring with newly created individual_wrappers
      for (const auto& parent : parents) {
        crossover(offspring, parent);
      }

      // Clear parents for future use
      parents.clear();

      // This will apply mutation to each child in offspring
      for (auto& child : offspring) {
        mutation(child);
      }

      if (props.is_survival_selection_active) {
        for (auto& child : offspring) {
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

    if (props.is_actor_reporter_active) {
      auto& actor_reporter = config->actor_reporter;

      self->send(actor_reporter, note_end::value, constants::now());
      self->send(actor_reporter, report_info::value, actor_phase::total, 0, 0);
    }

    if (props.is_individual_reporter_active) {
      auto& individual_reporter = config->individual_reporter;

      self->send(individual_reporter, report_population::value, population,
                 std::size_t { 0 }, std::size_t { 0 });
    }

    std::cout << "Finished sequential" << std::endl;
  }

 public:
  using base_driver::base_driver;

  void run() {
    actor_system_config cfg;
    actor_system system { cfg };
    scoped_actor self { system };
    configuration* conf { new configuration { system_props, user_props } };

    start_reporters<individual, fitness_value>(*conf, system, self);

    shared_config config { conf };
    fitness_evaluation_operator fitness_evaluation { config };
    initialization_operator initialization { config };
    crossover_operator crossover { config };
    mutation_operator mutation { config };
    parent_selection_operator parent_selection { config };
    survival_selection_operator survival_selection { config };
    elitism_operator elitism { config };
    global_temination_check termination_check { config };

    run_pga(self, config, fitness_evaluation, initialization, crossover,
            mutation, parent_selection, survival_selection, elitism,
            termination_check);

    stop_reporters(*conf, self);
  }
};

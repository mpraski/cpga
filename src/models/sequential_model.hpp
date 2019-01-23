#pragma once

#include <caf/all.hpp>
#include <core.hpp>

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
class sequential_model_driver : public base_single_machine_driver<individual, fitness_value> {
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

    couples<individual, fitness_value> parents;

    population<individual, fitness_value> main;
    population<individual, fitness_value> offspring;
    population<individual, fitness_value> elitists;

    parents.reserve(props.population_size / 2);
    main.reserve(props.population_size + props.elitists_number);
    offspring.reserve(props.population_size);
    elitists.reserve(props.elitists_number);

    if (props.is_generation_reporter_active) {
      self->send(config->generation_reporter, note_start::value, now(), island_0);
    }

    initialization(std::back_inserter(main));

    for (size_t g = 0; g < props.generations_number; ++g) {
      if (props.is_generation_reporter_active) {
        self->send(config->generation_reporter, note_start::value, now(), island_0);
      }

      for (auto &member : main) {
        member.second = fitness_evaluation(member.first);
      }

      if (props.is_elitism_active) {
        elitism(main, elitists);
      }

      // This will fill parents with individual_wrapper_pairs, each holding two copied individuals
      parent_selection(main, parents);

      // This will fill offspring with newly created individual_wrappers
      auto offspring_inserter{std::back_inserter(offspring)};
      for (const auto &couple : parents) {
        crossover(offspring_inserter, couple);
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

        survival_selection(main, offspring);
      }

      main.swap(offspring);
      offspring.clear();

      if (props.is_elitism_active) {
        main.insert(main.end(),
                    std::make_move_iterator(elitists.begin()),
                    std::make_move_iterator(elitists.end()));
        elitists.clear();
      }

      if (props.is_generation_reporter_active) {
        self->send(config->generation_reporter, note_end::value, now(), actor_phase::execute_generation, g, island_0);
      }

      log(self, "Generations so far: ", g);

      if (termination_check(main)) {
        break;
      }
    }

    for (auto &member : main) {
      member.second = fitness_evaluation(member.first);
    }

    if (props.is_generation_reporter_active) {
      auto &generation_reporter = config->generation_reporter;

      self->send(generation_reporter, note_end::value, now(),
                 actor_phase::total, props.generations_number, island_0);
    }

    if (props.is_individual_reporter_active) {
      auto &individual_reporter = config->individual_reporter;

      self->send(individual_reporter, report_population::value, main,
                 props.generations_number, island_0);
    }
  }

 public:
  using base_single_machine_driver<individual, fitness_value>::base_single_machine_driver;

  pga_model get_model() override {
    return pga_model::SEQUENTIAL;
  }

  void perform(shared_config &config, actor_system &system, scoped_actor &self) override {
    run_pga(self, config);
  }
};

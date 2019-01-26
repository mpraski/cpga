#pragma once

#include <algorithm>
#include <core.hpp>

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename crossover_operator,
    typename mutation_operator, typename parent_selection_operator,
    typename survival_selection_operator, typename elitism_operator>
struct grid_model_worker_state : public base_state {
  grid_model_worker_state() = default;
  explicit grid_model_worker_state(const shared_config &config)
      : base_state{config},
        fitness_evaluation{config, island_0},
        crossover{config, island_0},
        mutation{config, island_0},
        parent_selection{config, island_0},
        survival_selection{config, island_0},
        elitism{config, island_0} {
    offspring.reserve(config->system_props.population_size);
    elitists.reserve(config->system_props.elitists_number);
  }

  fitness_evaluation_operator fitness_evaluation;
  crossover_operator crossover;
  mutation_operator mutation;
  parent_selection_operator parent_selection;
  survival_selection_operator survival_selection;
  elitism_operator elitism;

  couples<individual, fitness_value> parents;
  population<individual, fitness_value> offspring;
  population<individual, fitness_value> elitists;

  inline void reset() noexcept {
    parents.clear();
    offspring.clear();
    elitists.clear();
  }
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator,
    typename crossover_operator,
    typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator,
    typename elitism_operator>
behavior grid_model_worker(
    stateful_actor<
        grid_model_worker_state<individual, fitness_value,
                                fitness_evaluation_operator, crossover_operator, mutation_operator,
                                parent_selection_operator, survival_selection_operator,
                                elitism_operator>> *self,
    const shared_config &config) {
  self->state = grid_model_worker_state<individual, fitness_value,
                                        fitness_evaluation_operator, crossover_operator, mutation_operator,
                                        parent_selection_operator, survival_selection_operator, elitism_operator>{
      config};

  return {
      [self](execute_computation, size_t gen, population<individual, fitness_value> &pop) {
        auto &state = self->state;
        auto &props = self->state.config->system_props;

        generation_message(self, note_start::value, now(), self->id());

        auto population = std::move(pop);
        state.reset();

        if (props.is_elitism_active) {
          state.elitism(population, state.elitists);
        }

        state.parent_selection(population, state.parents);

        auto inserter = std::back_inserter(state.offspring);
        for (const auto &couple : state.parents) {
          state.crossover(inserter, couple);
        }

        state.parents.clear();

        for (auto &child : state.offspring) {
          state.mutation(child);
        }

        if (props.is_survival_selection_active) {
          for (auto&[ind, value] : state.offspring) {
            value = state.fitness_evaluation(ind);
          }

          state.survival_selection(population, state.offspring);
        }

        population.swap(state.offspring);

        if (props.is_elitism_active) {
          population.insert(population.end(),
                            std::make_move_iterator(state.elitists.begin()),
                            std::make_move_iterator(state.elitists.end()));
          state.elitists.clear();
        }

        generation_message(self, note_end::value, now(), actor_phase::execute_computation, gen, self->id());

        return population;
      },
      [self](finish_worker) {
        system_message(self, "Quitting grid model worker (id: ", self->id(), ")");
        self->quit();
      }
  };
}

struct grid_model_dispatcher_state : public base_state {
  grid_model_dispatcher_state() = default;
  explicit grid_model_dispatcher_state(const shared_config &config, std::vector<actor> workers)
      : base_state{config},
        pool{workers.size()},
        counter{0},
        workers{std::move(workers)} {
  }

  inline auto get_worker() noexcept {
    return workers[counter++ % pool];
  }

  size_t pool;
  size_t counter;
  std::vector<actor> workers;
};

template<typename individual, typename fitness_value>
behavior grid_model_dispatcher(
    stateful_actor<grid_model_dispatcher_state> *self,
    grid_model_dispatcher_state state) {
  self->state = std::move(state);

  system_message(self, "Spawning grid model dispatcher");

  self->set_down_handler(
      [self](down_msg &down) {
        if (!down.reason) return;

        system_message(self, "Grid worker with actor id: ", down.source.id(), " died, respawning...");

        auto &workers = self->state.workers;
        workers.erase(std::remove_if(std::begin(workers),
                                     std::end(workers),
                                     [src = down.source](const auto &worker) { return src == worker; }));

        self->state.pool = workers.size();
      });

  return {
      [self](execute_computation atom, size_t gen, population<individual, fitness_value> &pop) {
        self->delegate(self->state.get_worker(), atom, gen, std::move(pop));
      },
      [self](finish) {
        for (const auto &worker : self->state.workers) {
          self->send(worker, finish_worker::value);
        }

        system_message(self, "Quitting grid model dispatcher");
        self->quit();
      }
  };
}

template<typename individual, typename fitness_value,
    typename initialization_operator,
    typename fitness_evaluation_operator>
struct grid_model_executor_state : public base_state {
  grid_model_executor_state() = default;
  grid_model_executor_state(const shared_config &config)
      : base_state{config},
        computation_done{0},
        computation_counter{0},
        current_generation{0},
        initialization{config, island_special},
        fitness_evaluation{config, island_special},
        random_nums(config->system_props.population_size),
        generator(now().time_since_epoch().count()) {
    main.reserve(config->system_props.population_size);
    result.reserve(config->system_props.population_size);
    std::iota(std::begin(random_nums), std::end(random_nums), size_t{});
  }

  size_t computation_done;
  size_t computation_counter;
  size_t current_generation;

  initialization_operator initialization;
  fitness_evaluation_operator fitness_evaluation;

  std::vector<size_t> random_nums;
  std::default_random_engine generator;
  population<individual, fitness_value> main;
  population<individual, fitness_value> result;
};

template<typename individual, typename fitness_value,
    typename initialization_operator,
    typename fitness_evaluation_operator>
behavior grid_model_executor(
    stateful_actor<
        grid_model_executor_state<individual, fitness_value,
                                  initialization_operator, fitness_evaluation_operator>> *self,
    const shared_config &config,
    const actor &dispatcher) {
  self->state =
      grid_model_executor_state<individual, fitness_value, initialization_operator, fitness_evaluation_operator>{
          config};
  self->monitor(dispatcher);

  system_message(self, "Spawning grid model executor");

  self->set_down_handler([self, dispatcher](down_msg &down) {
    if (down.source == dispatcher) {
      system_message(self, "Quitting executor as dispatcher already finished");
      self->quit();
    }
  });

  const auto &props = self->state.config->system_props;

  return {
      [=](init_population) {
        auto &state = self->state;

        generation_message(self, note_start::value, now(), island_special);
        generation_message(self, note_start::value, now(), island_special);

        state.initialization(std::back_inserter(state.main));
        self->send(self, execute_phase_1::value);

        generation_message(self,
                           note_end::value,
                           now(),
                           actor_phase::init_population,
                           state.current_generation,
                           island_special);
      },
      [=](execute_phase_1) {
        generation_message(self, note_start::value, now(), island_special);

        auto &state = self->state;
        auto islands = props.islands_number;
        auto generations = props.generations_number;
        auto rem = props.population_size % props.islands_number;
        auto times = props.population_size / props.islands_number;
        auto &gen = state.generator;
        auto &random_nums = state.random_nums;

        std::shuffle(random_nums.begin(), random_nums.end(), gen);

        for (auto&[ind, value] : state.main) {
          value = state.fitness_evaluation(ind);
        }

        for (size_t i = 0; i < props.population_size; i += times) {
          if (i + times + rem >= props.population_size) {
            times += rem;
          }

          population<individual, fitness_value> pop;
          for (size_t j = i; j < i + times; ++j) {
            pop.emplace_back(std::move(state.main[random_nums[j]]));
          }

          state.computation_counter = islands;

          self->request(
              dispatcher,
              timeout,
              execute_computation::value,
              self->state.current_generation,
              pop
          ).then(
              [self, generations](population<individual, fitness_value> &result) {
                self->state.result.insert(self->state.result.end(),
                                          std::make_move_iterator(result.begin()),
                                          std::make_move_iterator(result.end()));

                if (++self->state.computation_done == self->state.computation_counter) {
                  self->state.computation_done = 0;
                  self->state.main.clear();
                  self->state.main.swap(self->state.result);

                  generation_message(self,
                                     note_end::value,
                                     now(),
                                     actor_phase::execute_phase_1,
                                     self->state.current_generation,
                                     island_special);

                  if (++self->state.current_generation <= generations) {
                    self->send(self, execute_phase_1::value);
                  } else {
                    self->send(self, execute_phase_2::value);
                  }

                  log(self, "Generations so far: ", self->state.current_generation);
                }
              },
              [self, i](error &err) {
                system_message(self,
                               "Failed to execute computation for batch no: ",
                               i,
                               " with error code: ",
                               err.code());

                if (--self->state.computation_counter == 0) {
                  system_message(self, "Complete failure to perform computations, quitting...");
                  self->send(self, execute_phase_2::value);
                }
              }
          );
        }
      },
      [=](execute_phase_2) {
        auto &state = self->state;

        for (auto&[ind, value] : state.main) {
          value = state.fitness_evaluation(ind);
        }

        generation_message(self, note_end::value, now(), actor_phase::total, state.current_generation, island_special);
        individual_message(self, report_population::value, state.main, state.current_generation, island_special);

        self->send(dispatcher, finish::value);
      },
  };
}


#pragma once

#include <algorithm>
#include <random>
#include "../core.hpp"

/*
 * This file defines the behaviour of global model PGA.
 * Conceptually, the Driver (global_model_driver) initiates the computation
 * by spawning the Executor (global_model_executor) and Supervisor (global_model_supervisor).
 * Supervisor then spawns a number of Workers (global_model_worker) which perform fitness value
 * computation when requested. The Executor carries out the main loop of computation split into
 * distinct stages (init_population, execute_state_1, ... , finish) so that parallel calculation of
 * fitness values are possible.
 *
 * Technically, each actor (Executor, Supervisor, Worker) are composed of two parts:
 *  - state (e.g. 'struct global_model_worker_state') which defines the internal state an actor can manage
 *  - behaviour (e.g. 'behavior global_model_worker(...)') which defines message handlers of an actor, for instance
 *  clause '[=](init_population) { ... }' executes its body when an actor receives the init_population message.
 */

/*
 * WORKER
 *
 * Holds a fitness_evaluation_operator in its state, uses it
 * to perform fitness value evaluation when 'compute_fitness'
 * is received.
 */
template<typename fitness_evaluation_operator>
struct global_model_worker_state : public base_state {
  global_model_worker_state() = default;

  global_model_worker_state(const shared_config& config,
                            const fitness_evaluation_operator& feo)
      : base_state { config },
        fitness_evaluation { feo } {
  }

  fitness_evaluation_operator fitness_evaluation;
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator>
behavior global_model_worker(
    stateful_actor<global_model_worker_state<fitness_evaluation_operator>>* self,
    global_model_worker_state<fitness_evaluation_operator> state) {
  self->state = std::move(state);
  auto& f = self->state.fitness_evaluation;

  return {
    [f](compute_fitness, const individual& ind) -> fitness_value {
      return f(ind);
    }
  };
}

/*
 * SUPERVISOR
 *
 * Spawns and manages a number of workers, its only role is to
 * delegate the 'compute_fitness' message to a uniformly chosen
 * worker.
 */
struct global_model_supervisor_state : public base_state {
  std::size_t pool_size;
  std::vector<actor> workers;
  std::default_random_engine generator;
  std::uniform_int_distribution<std::size_t> distribution;
  std::function<std::size_t()> random_f;

  global_model_supervisor_state() = default;

  global_model_supervisor_state(const shared_config& config,
                                std::size_t pool_size)
      : base_state { config },
        pool_size { pool_size },
        generator { config->system_props.supervisor_seed },
        distribution { 0, pool_size - 1 },
        random_f { std::bind(distribution, generator) } {
    workers.reserve(pool_size);
  }

  inline auto random_worker() noexcept {
    return workers[random_f()];
  }
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator>
behavior global_model_supervisor(
    stateful_actor<global_model_supervisor_state>* self,
    global_model_supervisor_state state,
    const fitness_evaluation_operator& feo) {
  self->state = std::move(state);

  system_message(self, "Spawning global model supervisor");

  auto spawn_worker = [self, feo]() -> actor {
    system_message(self, "Spawning new global worker");
    return self->spawn<monitored + detached>(
        global_model_worker<individual, fitness_value,
        fitness_evaluation_operator>,
        global_model_worker_state<fitness_evaluation_operator> {self->state
          .config, feo});
  };

  for (std::size_t i = 0; i < self->state.pool_size; ++i)
    self->state.workers.emplace_back(spawn_worker());

  return {
    [self](compute_fitness cf, individual ind) {
      self->delegate(self->state.random_worker(), cf, std::move(ind));
    },
    [self, spawn_worker](const down_msg& down) {
      system_message(self, "Global worker with id: ", down.source.id(), " died, respawning...");

      auto& workers = self->state.workers;
      auto it = workers.begin();
      auto id = down.source.id();

      while(it != workers.end()) {
        if(it->id() == id) break;
        ++it;
      }

      workers.erase(it);
      workers.emplace_back(spawn_worker());
    },
  };
}

/*
 * EXECUTOR
 *
 * The bulk of PGA logic is implemented by the executor. It stores all the required
 * genetic operators as well as the individual and parent collections required at certain
 * stages of execution. It performs the next state of execution by sending an appropriate message
 * to itself - e.g. the 'self->send(self, execute_phase_1::value)' statement tells it to execute
 * the code defined in '[=](execute_phase_1) { ... }' after the population initialisation step is done.
 */
template<typename individual, typename fitness_value,
    typename initialization_operator, typename crossover_operator,
    typename mutation_operator, typename parent_selection_operator,
    typename global_temination_check, typename survival_selection_operator,
    typename elitism_operator>
struct global_model_executor_state : public base_state {
  global_model_executor_state() = default;

  global_model_executor_state(const shared_config& config,
                              initialization_operator io, crossover_operator co,
                              mutation_operator mo,
                              parent_selection_operator pso,
                              survival_selection_operator sso,
                              elitism_operator eo, global_temination_check gtc)
      : base_state { config },
        initialization { std::move(io) },
        crossover { std::move(co) },
        mutation { std::move(mo) },
        parent_selection { std::move(pso) },
        survival_selection { std::move(sso) },
        elitism { std::move(eo) },
        termination_check { std::move(gtc) },
        current_generation { 0 },
        current_island { 0 },
        compute_fitness_counter { 0 },
        population_size_counter { 0 },
        offspring_size_counter { 0 } {
    population.reserve(
        config->system_props.population_size
            + config->system_props.elitists_number);
    offspring.reserve(config->system_props.population_size);
    elitists.reserve(config->system_props.elitists_number);
  }

  initialization_operator initialization;
  crossover_operator crossover;
  mutation_operator mutation;
  parent_selection_operator parent_selection;
  survival_selection_operator survival_selection;
  elitism_operator elitism;
  global_temination_check termination_check;

  parent_collection<individual, fitness_value> parents;
  individual_collection<individual, fitness_value> population;
  individual_collection<individual, fitness_value> offspring;
  individual_collection<individual, fitness_value> elitists;

  std::size_t current_generation;
  std::size_t current_island;
  std::size_t compute_fitness_counter;
  std::size_t population_size_counter;
  std::size_t offspring_size_counter;
};

template<typename individual, typename fitness_value,
    typename initialization_operator, typename crossover_operator,
    typename mutation_operator, typename parent_selection_operator,
    typename global_temination_check, typename survival_selection_operator,
    typename elitism_operator>
behavior global_model_executor(
    stateful_actor<
        global_model_executor_state<individual, fitness_value,
            initialization_operator, crossover_operator, mutation_operator,
            parent_selection_operator, global_temination_check,
            survival_selection_operator, elitism_operator>>* self,
    global_model_executor_state<individual, fitness_value,
        initialization_operator, crossover_operator, mutation_operator,
        parent_selection_operator, global_temination_check,
        survival_selection_operator, elitism_operator> state,
    const actor& supervisor) {
  self->state = std::move(state);

  system_message(self, "Spawning global model executor");

  return {
    [self](init_population) {
      auto& state = self->state;
      auto& props = self->state.config->system_props;

      if(props.is_actor_reporter_active) {
        auto& actor_reporter = state.config->actor_reporter;

        self->send(actor_reporter, note_start::value, now());
        self->send(actor_reporter, note_start::value, now());
      }

      state.initialization(state.population);
      self->send(self, execute_phase_1::value);

      if(props.is_actor_reporter_active) {
        auto& actor_reporter = state.config->actor_reporter;

        self->send(actor_reporter, note_end::value, now());
        self->send(actor_reporter, report_info::value, actor_phase::init_population, state.current_generation, state.current_island);
      }
    },
    [self, supervisor](execute_phase_1) {
      auto& state = self->state;
      auto& props = self->state.config->system_props;

      if(props.is_actor_reporter_active) {
        self->send(state.config->actor_reporter, note_start::value, now());
      }

      state.population_size_counter = state.population.size();

      for (std::size_t i = 0; i < state.population_size_counter; ++i) {
        self->request(supervisor, timeout, compute_fitness::value, state.population[i].first).then(
            [=](fitness_value fv) {
              self->state.population[i].second = std::move(fv);

              if(++self->state.compute_fitness_counter == self->state.population_size_counter) {
                self->state.compute_fitness_counter = 0;

                self->send(self, execute_phase_2::value);

                if(self->state.config->system_props.is_actor_reporter_active) {
                  auto& actor_reporter = self->state.config->actor_reporter;

                  self->send(actor_reporter, note_end::value, now());
                  self->send(actor_reporter, report_info::value, actor_phase::execute_phase_1, self->state.current_generation, self->state.current_island);
                }
              }
            }
        );
      }
    },
    [self, supervisor](execute_phase_2) {
      auto& state = self->state;
      auto& props = self->state.config->system_props;

      if(props.is_actor_reporter_active) {
        self->send(state.config->actor_reporter, note_start::value, now());
      }

      if(props.is_elitism_active) {
        state.elitism(state.population, state.elitists);
      }

      state.parent_selection(state.population, state.parents);

      for(const auto& parent : state.parents) {
        state.crossover(state.offspring, parent);
      }

      state.parents.clear();

      for (auto& child : state.offspring) {
        state.mutation(child);
      }

      if(props.is_survival_selection_active) {
        state.offspring_size_counter = state.offspring.size();

        for (std::size_t i = 0; i < state.offspring_size_counter; ++i) {
          self->request(supervisor, timeout, compute_fitness::value, state.offspring[i].first).then(
              [=](fitness_value fv) {
                self->state.offspring[i].second = std::move(fv);

                if(++self->state.compute_fitness_counter == self->state.offspring_size_counter) {
                  self->state.compute_fitness_counter = 0;
                  self->state.survival_selection(self->state.population, self->state.offspring);

                  self->send(self, execute_phase_3::value);

                  if(self->state.config->system_props.is_actor_reporter_active) {
                    auto& actor_reporter = self->state.config->actor_reporter;

                    self->send(actor_reporter, note_end::value, now());
                    self->send(actor_reporter, report_info::value, actor_phase::execute_phase_2, self->state.current_generation, self->state.current_island);
                  }
                }
              }
          );
        }
      } else {
        self->send(self, execute_phase_3::value);

        if(props.is_actor_reporter_active) {
          auto& actor_reporter = state.config->actor_reporter;

          self->send(actor_reporter, note_end::value, now());
          self->send(actor_reporter, report_info::value, actor_phase::execute_phase_2, state.current_generation, state.current_island);
        }
      }
    },
    [self](execute_phase_3) {
      auto& state = self->state;
      auto& props = self->state.config->system_props;

      if(props.is_actor_reporter_active) {
        self->send(state.config->actor_reporter, note_start::value, now());
      }

      state.population.swap(state.offspring);
      state.offspring.clear();

      if(props.is_elitism_active) {
        state.population.insert(state.population.end(),
            std::make_move_iterator(state.elitists.begin()),
            std::make_move_iterator(state.elitists.end()));
        state.elitists.clear();
      }

      if(++state.current_generation == props.generations_number
          || state.termination_check(state.population)) {
        self->send(self, finish::value);
      } else {
        self->send(self, execute_phase_1::value);
      }

      if(props.is_actor_reporter_active) {
        auto& actor_reporter = state.config->actor_reporter;

        self->send(actor_reporter, note_end::value, now());
        self->send(actor_reporter, report_info::value, actor_phase::execute_phase_3, state.current_generation, state.current_island);
      }
    },
    [self](finish) {
      auto& state = self->state;
      auto& props = self->state.config->system_props;

      if(props.is_actor_reporter_active) {
        auto& actor_reporter = state.config->actor_reporter;

        self->send(actor_reporter, note_end::value, now());
        self->send(actor_reporter, report_info::value, actor_phase::total, state.current_generation, state.current_island);
      }

      if(props.is_individual_reporter_active) {
        auto& individual_reporter = state.config->individual_reporter;

        self->send(individual_reporter, report_population::value, state.population, state.current_generation, state.current_island);
      }

      self->quit();
    },
  };
}

/*
 * DRIVER
 *
 * The driver is responsible for starting the actor system, spawning
 * the Executor and Supervisor, spawning and configuring the reporters,
 * running the model by sending the initial message to the Executor, waiting for
 * the Executor to finish (e.g. the PGA computation concluded) and cleaning up.
 */
template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator, typename survival_selection_operator,
    typename elitism_operator, typename global_temination_check>
class global_model_driver : private base_driver {
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

    auto cores = std::max(std::thread::hardware_concurrency() - 2, 2u);

    auto supervisor = system.spawn(
        global_model_supervisor<individual, fitness_value,
            fitness_evaluation_operator>,
        global_model_supervisor_state { config, cores }, fitness_evaluation);

    auto executor = system.spawn(
        global_model_executor<individual, fitness_value,
            initialization_operator, crossover_operator, mutation_operator,
            parent_selection_operator, global_temination_check,
            survival_selection_operator, elitism_operator>,
        global_model_executor_state<individual, fitness_value,
            initialization_operator, crossover_operator, mutation_operator,
            parent_selection_operator, global_temination_check,
            survival_selection_operator, elitism_operator> { config, std::move(
            initialization), std::move(crossover), std::move(mutation),
            std::move(parent_selection), std::move(survival_selection),
            std::move(elitism), std::move(termination_check) },
        supervisor);

    self->send(executor, init_population::value);
    self->wait_for(executor);

    // Quit reporters and supervisor
    stop_reporters(*conf, self);
    anon_send_exit(supervisor, exit_reason::user_shutdown);
  }
};

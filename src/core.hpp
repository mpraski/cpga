#pragma once

#include "caf/all.hpp"
#include "reporter.hpp"
#include "atoms.hpp"
#include <any>

using namespace caf;

/*
 * user_properties is a map dedicated for
 * storage of arbitrary user-defined data (necessary for passing
 * custom arguments to some genetic operators)
 */
using user_properties = std::unordered_map<std::string, std::any>;

/*
 * system_properties struct defined parameters commonly used
 * by the framework.
 */
struct system_properties {
  std::size_t population_size;
  std::size_t individual_size;
  std::size_t islands_number;
  std::size_t generations_number;
  std::size_t elitists_number;
  std::size_t migration_period;
  std::size_t migration_quota;
  bool is_elitism_active;
  bool is_survival_selection_active;
  bool is_migration_active;
  bool can_repeat_individual_elements;
  bool add_island_no_to_seed;
  unsigned long initialization_seed;
  unsigned long mutation_seed;
  unsigned long crossover_seed;
  unsigned long parent_selection_seed;
  unsigned long survival_selection_seed;
  unsigned long supervisor_seed;
  unsigned long migration_seed;
  double mutation_probability;

  bool is_system_reporter_active;
  std::string system_reporter_log;

  bool is_generation_reporter_active;
  std::string generation_reporter_log;

  bool is_individual_reporter_active;
  std::string individual_reporter_log;
};

/*
 * configuration struct represents the user-defined
 * settings as well as some utitlites (access to reporter actors)
 */
struct configuration {
  configuration(const system_properties& system_props,
                const user_properties& user_props);

  system_properties system_props;
  user_properties user_props;

  // reporter actor handles
  actor system_reporter;
  actor generation_reporter;
  actor individual_reporter;
};

using shared_config = std::shared_ptr<const configuration>;

struct base_state {
  base_state() = default;
  base_state(const shared_config& config);

  shared_config config;
};

struct base_operator : public base_state {
  base_operator() = default;
  base_operator(const shared_config& config, island_id island_no);

  island_id island_no;

  inline auto get_seed(unsigned long seed) const noexcept {
    if (config->system_props.add_island_no_to_seed) {
      seed += island_no;
    }
    return seed;
  }
};

template<typename individual, typename fitness_value>
class base_driver {
 private:
  configuration* conf;
  shared_config config;
  system_properties system_props;
  user_properties user_props;

  void start_reporters(configuration& conf, actor_system& system,
                       scoped_actor& self) const {
    if (system_props.is_system_reporter_active) {
      if (system_props.system_reporter_log.empty()) {
        throw std::runtime_error("system_reporter_log is empty");
      }

      conf.system_reporter = system.spawn(system_reporter);  // @suppress("Invalid arguments")

      self->send(conf.system_reporter, init_reporter::value,
                 system_props.system_reporter_log, constants::SYSTEM_HEADERS);

      system_message(self, conf.system_reporter, "Spawning reporters");
    }

    if (system_props.is_generation_reporter_active) {
      if (system_props.generation_reporter_log.empty()) {
        throw std::runtime_error("actor_reporter_log is empty");
      }

      conf.generation_reporter = system.spawn(time_reporter);  // @suppress("Invalid arguments")

      self->send(conf.generation_reporter, init_reporter::value,
                 system_props.generation_reporter_log, constants::TIME_HEADERS);
    }

    if (system_props.is_individual_reporter_active) {
      if (system_props.individual_reporter_log.empty()) {
        throw std::runtime_error("actor_reporter_log is empty");
      }

      conf.individual_reporter = system.spawn(
          individual_reporter<individual, fitness_value>);

      self->send(conf.individual_reporter, init_reporter::value,
                 system_props.individual_reporter_log,
                 constants::INDIVIDUAL_HEADERS);
    }
  }

  void stop_reporters(configuration& conf, scoped_actor& self) const {
    if (system_props.is_system_reporter_active) {
      system_message(self, conf.system_reporter, "Stopping reporters");

      self->send(conf.system_reporter, exit_reporter::value);
    }

    if (system_props.is_generation_reporter_active) {
      self->send(conf.generation_reporter, exit_reporter::value);
    }

    if (system_props.is_individual_reporter_active) {
      self->send(conf.individual_reporter, exit_reporter::value);
    }
  }
 protected:
  virtual void perform(shared_config& config, actor_system& system,
                       scoped_actor& self) = 0;
 public:
  base_driver(const system_properties& system_props,
              const user_properties& user_props)
      : conf { new configuration { system_props, user_props } },
        config { conf },
        system_props { system_props },
        user_props { user_props } {
  }

  virtual ~base_driver() {
  }

  void run() {
    actor_system_config cfg;
    actor_system system { cfg };
    scoped_actor self { system };

    start_reporters(*conf, system, self);
    perform(config, system, self);
    stop_reporters(*conf, self);
  }
};

// Default implementations of optional operators (for default template arguments)
template<typename individual, typename fitness_value>
struct default_survival_selection_operator : base_operator {
  using base_operator::base_operator;

  void operator()(
      individual_collection<individual, fitness_value>& parents,
      individual_collection<individual, fitness_value>& offspring) const
          noexcept {
  }
};

template<typename individual, typename fitness_value>
struct default_elitism_operator : base_operator {
  using base_operator::base_operator;

  void operator()(
      individual_collection<individual, fitness_value>& population,
      individual_collection<individual, fitness_value>& elitists) const
          noexcept {
  }
};

template<typename individual, typename fitness_value>
struct default_migration_operator : base_operator {
  using base_operator::base_operator;

  migration_payload<individual, fitness_value> operator()(
      island_id from,
      individual_collection<individual, fitness_value>& population) const
          noexcept {
    return {};
  }
};

template<typename individual, typename fitness_value>
struct default_global_temination_check : base_operator {
  using base_operator::base_operator;

  bool operator()(
      const individual_collection<individual, fitness_value>& population) const
          noexcept {
    return false;
  }
};

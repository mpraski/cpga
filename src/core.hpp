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
  bool is_elitism_active;
  bool is_survival_selection_active;
  bool can_repeat_individual_elements;
  double initialization_seed;
  double mutation_seed;
  double crossover_seed;
  double parent_selection_seed;
  double survival_selection_seed;
  double mutation_probability;
  double supervisor_seed;

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
  system_properties system_props;
  user_properties user_props;

  // reporter actor handles
  actor system_reporter;
  actor generation_reporter;
  actor individual_reporter;

  configuration(const system_properties& system_props,
                const user_properties& user_props);
};

using shared_config = std::shared_ptr<const configuration>;

struct base_state {
  base_state() = default;

  base_state(const shared_config& config);

  shared_config config;
};

class base_driver {
 protected:
  system_properties system_props;
  user_properties user_props;
 public:
  base_driver(const system_properties& system_props,
              const user_properties& user_props);

  template<typename individual, typename fitness_value>
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

  void stop_reporters(configuration& conf, scoped_actor& self) const;
};

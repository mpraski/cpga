//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_DATA_H
#define GENETIC_ACTOR_DATA_H

#include <any>
#include <common.hpp>
#include "message_bus.hpp"

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
  size_t population_size;
  size_t individual_size;
  size_t islands_number;
  size_t generations_number;
  size_t elitists_number;
  size_t migration_period;
  size_t migration_quota;
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
 * settings as well as some utilities (access to reporter actors)
 */
struct configuration {
  configuration(const system_properties &system_props,
                const user_properties &user_props);

  configuration(const system_properties &system_props,
                const user_properties &user_props,
                actor &generation_reporter,
                actor &individual_reporter,
                actor &system_reporter,
                message_bus &&bus);

  system_properties system_props;
  user_properties user_props;

  // reporter actor handles
  actor generation_reporter;
  actor individual_reporter;
  actor system_reporter;

  // Simple message bus for worker/dispatcher communication
  message_bus bus;
};

using shared_config = std::shared_ptr<const configuration>;

template<typename... Args>
inline auto make_shared_config(Args &&... args) {
  return std::make_shared<const configuration>(std::forward<Args>(args)...);
}

#endif //GENETIC_ACTOR_DATA_H

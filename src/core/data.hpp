//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_DATA_H
#define GENETIC_ACTOR_DATA_H

#include <any>
#include <common.hpp>
#include "message_bus.hpp"

/**
 * @brief This alias to a map between string and any is dedicated for
 * storage of arbitrary user-defined data (necessary for passing
 * custom arguments to some genetic operators)
 */
using user_properties = std::unordered_map<std::string, std::any>;

/**
 * @brief This struct defines parameters commonly used
 * by the framework.
 */
struct system_properties {
  size_t total_population_size;
  size_t population_size;
  size_t individual_size;
  size_t islands_number;
  size_t generations_number;
  size_t elitists_number;
  size_t migration_period;
  size_t migration_quota;
  bool is_grid_model;
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
  unsigned long migration_seed;
  double crossover_probability;
  double mutation_probability;

  bool is_system_reporter_active;
  std::string system_reporter_log;

  bool is_generation_reporter_active;
  std::string generation_reporter_log;

  bool is_individual_reporter_active;
  std::string individual_reporter_log;

  pga_model model;

  /**
   * @brief Apply model-specific configuration.
   * @details This function primarily computes the island population size
   * for the user. It should be called after all parameters are set.
   */
  inline void compute_population_size() {
    if (model == pga_model::ISLAND) {
      if (total_population_size && islands_number) {
        population_size = total_population_size / islands_number;
      } else if (!population_size) {
        throw std::runtime_error("Either total_population_size != 0 && islands_number != 0 or population_size != 0");
      }
    } else if (model == pga_model::GRID) {
      if (total_population_size) {
        population_size = total_population_size;
      } else {
        throw std::runtime_error("total_population_size > 0 in grid model");
      }
    }
  }

  inline void island_model() {
    model = pga_model::ISLAND;
  }

  inline void grid_model() {
    model = pga_model::GRID;
  }

  inline void global_model() {
    model = pga_model::GLOBAL;
  }

  inline void sequential_model() {
    model = pga_model::SEQUENTIAL;
  }

  system_properties();
};

/**
 * @brief This struct encapsulates system- and user-specific
 * settings as well as access to certain utilities (reporter actors and
 * message bus).
 */
struct configuration {
  configuration(const system_properties &system_props,
                const user_properties &user_props,
                message_bus &&bus);

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

/**
 * @brief This shared pointer is the only way configuration can be accessed
 * during model runtime. It needs to be safely published (immutable) to ensure
 * tread safety.
 */
using shared_config = std::shared_ptr<const configuration>;

template<typename... Args>
inline auto make_shared_config(Args &&... args) {
  return std::make_shared<const configuration>(std::forward<Args>(args)...);
}

template<typename A>
inline void show_model_info(A &&self, const system_properties &props) {
  log(self, "** PGA info **");
  log(self, "-- Model: ", constants::PGA_MODEL_MAP[to_underlying(props.model)]);
  log(self, "-- Generations: ", props.generations_number);
  switch (props.model) {
    case pga_model::ISLAND:log(self, "-- Total initial population size: ", props.total_population_size);
      log(self, "-- Islands: ", props.islands_number);
      log(self, "-- Per island initial population size: ", props.population_size);
      log(self, "-- Migration quota: ", props.migration_quota);
      log(self, "-- Migration period: ", props.migration_period, " generations");
      break;
    case pga_model::GRID:log(self, "-- Total initial population size: ", props.population_size);
      log(self, "-- Grid workers: ", props.islands_number);
      break;
    case pga_model::GLOBAL:
    case pga_model::SEQUENTIAL:log(self, "-- Total initial population size: ", props.population_size);
      break;
  }
  std::vector<std::string> reporters;
  if (props.is_generation_reporter_active)
    reporters.push_back("generation reporter (" + props.generation_reporter_log + ")");
  if (props.is_individual_reporter_active)
    reporters.push_back("individual reporter (" + props.individual_reporter_log + ")");
  if (props.is_system_reporter_active)
    reporters.push_back("system reporter (" + props.system_reporter_log + ")");
  log(self, "-- Reporters: ", join(reporters, ", "));
  log(self, "** Starting execution now **");
}

#endif //GENETIC_ACTOR_DATA_H

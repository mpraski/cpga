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
  std::size_t island_number;
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

  bool is_actor_reporter_active;
  std::string actor_reporter_log;
};

/*
 * configuration struct represents the user-defined
 * settings as well as some utitlites (access to reporter actors)
 */
struct configuration {
  system_properties system_props;
  user_properties user_props;
  actor actor_reporter;

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
 private:
  std::vector<std::string> build_actor_reporter_headers() const;
 public:
  base_driver(const system_properties& system_props,
              const user_properties& user_props);

  void start_reporters(configuration& conf, actor_system& system,
                       scoped_actor& self) const;

  void stop_reporters(configuration& conf, scoped_actor& self) const;

  system_properties system_props;
  user_properties user_props;
};

// aliases for common data structures
template<typename individual, typename fitness_value>
using individual_wrapper = std::pair<individual, fitness_value>;

template<typename individual, typename fitness_value>
using individual_wrapper_pair = std::pair<individual_wrapper<individual, fitness_value>, individual_wrapper<individual, fitness_value>>;

template<typename individual, typename fitness_value>
using individual_collection = std::vector<individual_wrapper<individual, fitness_value>>;

template<typename individual, typename fitness_value>
using parent_collection = std::vector<individual_wrapper_pair<individual, fitness_value>>;

#pragma once

#include <any>
#include <functional>
#include <caf/all.hpp>
#include <caf/io/all.hpp>
#include <reporter.hpp>
#include <atoms.hpp>

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
                const user_properties &user_props) : system_props{system_props},
                                                     user_props{user_props} {
  }

  configuration(const system_properties &system_props,
                const user_properties &user_props,
                const actor &generation_reporter,
                const actor &individual_reporter,
                const actor &system_reporter) : system_props{system_props},
                                                user_props{user_props},
                                                system_reporter{system_reporter},
                                                generation_reporter{generation_reporter},
                                                individual_reporter{individual_reporter} {
  }

  system_properties system_props;
  user_properties user_props;

  // reporter actor handles
  actor system_reporter;
  actor generation_reporter;
  actor individual_reporter;
};

using shared_config = std::shared_ptr<const configuration>;

template<typename... Args>
inline auto make_shared_config(Args &&... args) {
  return std::make_shared<const configuration>(std::forward<Args>(args)...);
}

struct base_state {
  base_state() = default;
  explicit base_state(const shared_config &config) : config{config} {
  }

  shared_config config;
};

struct base_operator : public base_state {
  base_operator() = default;
  base_operator(const shared_config &config, island_id island_no) : base_state{config},
                                                                    island_no{island_no} {
  }

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
  std::shared_ptr<configuration> config;
  system_properties system_props;
  user_properties user_props;

  void start_reporters(std::shared_ptr<configuration> &config, actor_system &system,
                       scoped_actor &self) const {
    if (system_props.is_system_reporter_active) {
      if (system_props.system_reporter_log.empty()) {
        throw std::runtime_error("system_reporter_log is empty");
      }

      config->system_reporter = system.spawn(system_reporter);

      self->send(config->system_reporter, init_reporter::value,
                 system_props.system_reporter_log, constants::SYSTEM_HEADERS);

      system_message(self, config->system_reporter, "Spawning reporters");
    }

    if (system_props.is_generation_reporter_active) {
      if (system_props.generation_reporter_log.empty()) {
        throw std::runtime_error("actor_reporter_log is empty");
      }

      config->generation_reporter = system.spawn(time_reporter);

      self->send(config->generation_reporter, init_reporter::value,
                 system_props.generation_reporter_log, constants::TIME_HEADERS);
    }

    if (system_props.is_individual_reporter_active) {
      if (system_props.individual_reporter_log.empty()) {
        throw std::runtime_error("actor_reporter_log is empty");
      }

      config->individual_reporter = system.spawn(
          individual_reporter<individual, fitness_value>);

      self->send(config->individual_reporter, init_reporter::value,
                 system_props.individual_reporter_log,
                 constants::INDIVIDUAL_HEADERS);
    }
  }

  void stop_reporters(std::shared_ptr<configuration> &config, scoped_actor &self) const {
    if (system_props.is_system_reporter_active) {
      system_message(self, config->system_reporter, "Stopping reporters");

      self->send(config->system_reporter, exit_reporter::value);
    }

    if (system_props.is_generation_reporter_active) {
      self->send(config->generation_reporter, exit_reporter::value);
    }

    if (system_props.is_individual_reporter_active) {
      self->send(config->individual_reporter, exit_reporter::value);
    }
  }
 protected:
  virtual void perform(shared_config &config, actor_system &system,
                       scoped_actor &self) = 0;
 public:
  base_driver(const system_properties &system_props,
              const user_properties &user_props)
      : config{std::make_shared<configuration>(system_props, user_props)},
        system_props{system_props},
        user_props{user_props} {
  }

  virtual ~base_driver() = default;

  void run() {
    actor_system_config cfg;
    actor_system system{cfg};
    scoped_actor self{system};

    shared_config const_config{std::const_pointer_cast<const configuration>(config)};

    start_reporters(config, system, self);
    perform(const_config, system, self);
    stop_reporters(config, self);
  }
};

// Default implementations of optional operators (for default template arguments)
template<typename individual, typename fitness_value>
struct default_survival_selection_operator : base_operator {
  using base_operator::base_operator;

  void operator()(
      individual_collection<individual, fitness_value> &parents,
      individual_collection<individual, fitness_value> &offspring) const
  noexcept {
  }
};

template<typename individual, typename fitness_value>
struct default_elitism_operator : base_operator {
  using base_operator::base_operator;

  void operator()(
      individual_collection<individual, fitness_value> &population,
      individual_collection<individual, fitness_value> &elitists) const
  noexcept {
  }
};

template<typename individual, typename fitness_value>
struct default_migration_operator : base_operator {
  using base_operator::base_operator;

  migration_payload<individual, fitness_value> operator()(
      island_id from,
      individual_collection<individual, fitness_value> &population) const
  noexcept {
    return {};
  }
};

std::vector<actor> bind_remote_workers(actor_system &system, const std::vector<worker_node_info> &infos);

std::tuple<actor, actor, actor> bind_remote_reporters(actor_system &system, const reporter_node_info &info);

template<typename individual, typename fitness_value>
struct default_global_termination_check : base_operator {
  using base_operator::base_operator;

  bool operator()(
      const individual_collection<individual, fitness_value> &population) const
  noexcept {
    return false;
  }
};

class base_cluster_driver {
 private:
  template<typename T>
  static auto create_counter(T &&start) {
    return [c = start]() mutable {
      return c++;
    };
  }

  static auto remote_group(const std::string &group, const std::string &host, uint16_t port) {
    return str(group, "@", host, ":", port);
  }

  template<typename Gen, typename Cond, typename Time = std::chrono::milliseconds>
  static auto poll(Gen &&generator,
                   Cond &&condition,
                   size_t failed_attempts = 60,
                   Time &&period = std::chrono::seconds(1)) {
    auto checker = [&, c = size_t{0}]() mutable {
      return std::make_pair(generator(), ++c != failed_attempts);
    };

    while (true) {
      if (auto[result, quota] = checker(); !condition(result)) {
        if (quota) {
          std::this_thread::sleep_for(period);
        } else {
          throw std::runtime_error(str("polling did not succeed within ", failed_attempts, " tries"));
        }
      } else {
        return result;
      }
    }
  }
 protected:
  system_properties system_props;
  user_properties user_props;
  cluster_properties cluster_props;

  auto make_reporter_port_factory() const {
    return create_counter(cluster_props.reporter_range_start);
  }

  auto make_worker_port_factory() const {
    return create_counter(cluster_props.worker_range_start);
  }

  auto wait_for_master_node(io::middleman &m) {
    auto &host = cluster_props.master_node_host;
    auto &port = cluster_props.master_node_port;

    return *poll([&] { return m.remote_actor(host, port); }, identity{});
  }

  auto wait_for_node_group(actor_system &s) {
    auto &host = cluster_props.master_node_host;
    auto &port = cluster_props.master_group_port;

    return *poll([&] { return s.groups().get("remote", remote_group(constants::NODE_GROUP, host, port)); }, identity{});
  }

  virtual void perform(scoped_actor &self) = 0;
 public:
  base_cluster_driver(const system_properties &system_props,
                      const user_properties &user_props,
                      const cluster_properties &cluster_props) : system_props{system_props},
                                                                 user_props{user_props},
                                                                 cluster_props{cluster_props} {}
  virtual ~base_cluster_driver() {}

  void run(actor_system &system) {
    scoped_actor self{system};
    perform(self);
  }
};

struct base_cluster_state {
  base_cluster_state() = default;
  explicit base_cluster_state(const cluster_properties &cluster_props) : cluster_props{cluster_props} {}

  cluster_properties cluster_props;
};
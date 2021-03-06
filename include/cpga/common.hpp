#pragma once

#include <algorithm>
#include <random>
#include <chrono>
#include <type_traits>
#include <forward_list>
#include <thread>
#include <caf/all.hpp>
#include <caf/io/all.hpp>
#include "atoms.hpp"

namespace cpga {
using namespace caf;
using namespace caf::io;
using namespace cpga::atoms;

// aliases for common data structures
using island_id = size_t;

template<typename individual_value>
using sequence = std::vector<individual_value>;

template<typename individual, typename fitness_value>
using wrapper = std::pair<individual, fitness_value>;

template<typename individual, typename fitness_value>
using wrapper_pair = std::pair<wrapper<individual, fitness_value>,
                               wrapper<individual, fitness_value>>;

template<typename individual, typename fitness_value>
using population = std::vector<wrapper<individual, fitness_value>>;

template<typename individual, typename fitness_value>
using couples = std::vector<wrapper_pair<individual, fitness_value>>;

template<typename individual, typename fitness_value>
using migration_payload = std::vector<std::pair<island_id, wrapper<individual, fitness_value>>>;

template<typename individual, typename fitness_value>
using inserter = std::back_insert_iterator<population<individual, fitness_value>>;

// Commonly used data
namespace strings {
const constexpr char POSSIBLE_VALUES[] = "possible_initialization_values";
const constexpr char STABLE_REQUIRED[] = "stable_required";
const constexpr char MINIMUM_AVERAGE[] = "minimum_average";
const constexpr char CSV_FILE[] = "csv_file";
const constexpr char N_ROWS[] = "n_rows";
const constexpr char N_COLS[] = "n_cols";
const constexpr char N_FOLDS[] = "n_folds";
const constexpr char MUTATION_RANGE_C[] = "mutation_range_c";
const constexpr char MUTATION_RANGE_GAMMA[] = "mutation_range_gamma";
const constexpr char RANGE_C[] = "range_c";
const constexpr char RANGE_GAMMA[] = "range_gamma";
const constexpr char NODE_GROUP[] = "node_group";
const constexpr char MESSAGE_BUS_GROUP[] = "message_bus_group";

const constexpr char *const ACTOR_PHASE_MAP[] = {"init_population",
                                                 "execute_phase_1",
                                                 "execute_phase_2",
                                                 "execute_phase_3",
                                                 "finish",
                                                 "total",
                                                 "execute_generation",
                                                 "execute_computation"};

const constexpr char *const PGA_MODEL_MAP[] = {"Global", "Island", "Grid", "Sequential"};

const std::vector<std::string> SYSTEM_HEADERS{"Time", "Message"};
const std::vector<std::string> TIME_HEADERS{"Start", "End", "Total (ms)", "Phase", "Generation", "Island"};
const std::vector<std::string> INDIVIDUAL_HEADERS{"Generation", "Island", "Individual", "Fitness value"};
}

const constexpr auto timeout = std::chrono::seconds(10);
const constexpr auto island_0 = island_id{0};
const constexpr auto island_special = std::numeric_limits<island_id>::max();

inline auto now() noexcept {
  return std::chrono::high_resolution_clock::now();
}

inline auto recommended_worker_number() noexcept {
  return std::max(std::thread::hardware_concurrency() - 2, 2u);
}

template<typename E>
constexpr auto to_underlying(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

template<typename T, typename ... Ts>
constexpr auto is_same() noexcept {
  return (std::is_same_v<T, Ts> || ...);
}

template<typename T>
constexpr auto is_size_constructible() noexcept {
  using V = typename T::value_type;
  return is_same<T, std::vector<V>, std::list<V>, std::forward_list<V>, std::deque<V>>();
}

template<typename T, typename V = typename T::value_type>
auto join(const T &elements, const char *const delimiter = ",") {
  std::ostringstream os;
  auto b = std::begin(elements), e = std::end(elements);

  if (b != e) {
    std::copy(b, std::prev(e), std::ostream_iterator<V>(os, delimiter));
    b = std::prev(e);
  }
  if (b != e) {
    os << *b;
  }

  return os.str();
}

struct identity {
  template<typename T>
  constexpr T &&operator()(T &&t) const noexcept {
    return std::forward<T>(t);
  }
};

struct worker_node_info {
  std::string host;
  std::vector<uint16_t> worker_ports;
};

std::ostream &operator<<(std::ostream &os, const worker_node_info &x);

template<class Inspector>
typename Inspector::result_type inspect(Inspector &f, worker_node_info &x) {
  return f(meta::type_name("worker_node_info"), x.host, x.worker_ports);
}

struct reporter_node_info {
  std::string host;
  uint16_t system_reporter_port;
  uint16_t generation_reporter_port;
  uint16_t individual_reporter_port;

  reporter_node_info() = default;
  reporter_node_info(const std::string &host) : host{host},
                                                system_reporter_port{0},
                                                generation_reporter_port{0},
                                                individual_reporter_port{0} {}
};

std::ostream &operator<<(std::ostream &os, const reporter_node_info &x);

template<class Inspector>
typename Inspector::result_type inspect(Inspector &f, reporter_node_info &x) {
  return f(meta::type_name("reporter_node_info"),
           x.host,
           x.system_reporter_port,
           x.generation_reporter_port,
           x.individual_reporter_port);
}

enum class actor_phase {
  init_population,
  execute_phase_1,
  execute_phase_2,
  execute_phase_3,
  finish,
  total,
  execute_generation,
  execute_computation
};

enum class cluster_mode {
  MASTER, WORKER, REPORTER
};

enum class pga_model {
  GLOBAL, ISLAND, GRID, SEQUENTIAL
};

/**
 * @brief This class represents program configuration, where public members
 * are populated by appropriate command line arguments to the main executable.
 * @note One needs to invoke the CLUSTER_CONFIG macro defined below in order for the
 * definition of this class to be complete.
 * @see CLUSTER_CONFIG
 */
class cluster_properties : public actor_system_config {
 public:
  std::string master_node_host;
  std::string this_node_host;
  uint16_t master_node_port;
  uint16_t master_group_port;
  uint16_t reporter_range_start;
  uint16_t worker_range_start;
  size_t expected_worker_nodes;
  std::string _mode;
  size_t core_number = 1;
  std::string file_path = "/";

  cluster_properties();

  cluster_properties(const cluster_properties &props) : actor_system_config{},
                                                        master_node_host{props.master_node_host},
                                                        this_node_host{props.this_node_host},
                                                        master_node_port{props.master_node_port},
                                                        master_group_port{props.master_group_port},
                                                        reporter_range_start{props.reporter_range_start},
                                                        worker_range_start{props.worker_range_start},
                                                        expected_worker_nodes{props.expected_worker_nodes},
                                                        _mode{props._mode},
                                                        core_number{props.core_number},
                                                        file_path{props.file_path} {}
  cluster_properties &operator=(cluster_properties &&props) {
    master_node_host = std::move(props.master_node_host);
    this_node_host = std::move(props.this_node_host);
    master_node_port = props.master_node_port;
    master_group_port = props.master_group_port;
    reporter_range_start = props.reporter_range_start;
    worker_range_start = props.worker_range_start;
    expected_worker_nodes = props.expected_worker_nodes;
    _mode = std::move(props._mode);
    core_number = props.core_number;
    file_path = std::move(props.file_path);
    return *this;
  }

  inline cluster_mode mode() const {
    if (_mode == "MASTER") return cluster_mode::MASTER;
    if (_mode == "WORKER") return cluster_mode::WORKER;
    if (_mode == "REPORTER") return cluster_mode::REPORTER;

    throw std::runtime_error("unspecified cluster mode: '" + _mode + "'");
  }
};

/**
 * @brief This macro turned out to be the easiest way of configuring the
 * custom message types (CAF requires us to do so, since we're passing not only atoms but
 * individuals, fitness_values and migration payloads etc.) Since custom message types can only
 * be declared in a class inheriting from actor_system_config I've decided to create a macro expanding
 * to the implementation of default constructor of cluster_properties (it's the fitting place to add types).
 * @note The used has to invoke this macro with their intended individual and fitness_value types, otherwise they won't
 * even be able to launch the program with desired command line params which are also declared here.
 */
#define CLUSTER_CONFIG(IND, FIT) \
cluster_properties::cluster_properties() { \
  add_message_type<worker_node_info>("worker_node_info"); \
  add_message_type<reporter_node_info>("reporter_node_info"); \
  add_message_type<actor_phase>("actor_phase"); \
  add_message_type<IND>("IND"); \
  add_message_type<FIT>("FIT"); \
  add_message_type<std::pair<IND, FIT>>("std::pair<IND, FIT>"); \
  add_message_type<std::pair<std::pair<IND, FIT>, \
                             std::pair<IND, FIT>>>("std::pair<std::pair<IND, FIT>, std::pair<IND, FIT>"); \
  add_message_type<std::pair<size_t, std::pair<IND, FIT>>>("std::pair<size_t, std::pair<IND, FIT>>"); \
 \
  opt_group{custom_options_, "global"} \
      .add(master_node_host, "master-node-host,mnh", "set the host name for the master node") \
      .add(this_node_host,"this-node-host,tnh", "set the host name for the current node") \
      .add(master_node_port, "master-node-port,mnp", "set the port for the master node") \
      .add(master_group_port, "master-group-port,mgp", "set the port for master node group") \
      .add(reporter_range_start, "reporter-range-start,rrs", "set the port range start for reporter actors") \
      .add(worker_range_start, "worker-range-start,rrs", "set the port range start for worker actors") \
      .add(expected_worker_nodes, "expected-worker-nodes,ewn", "set the expected number of worker nodes") \
      .add(_mode, "mode,m", "set mode of operation (SERVER, WORKER or REPORTER)") \
      .add(core_number, "cores,j", "set number of cores used for computation on this machine") \
      .add(file_path, "file,f", "the path to input data file"); \
} \

template<typename T, typename ... Ts>
auto str(T &&t, Ts &&... ts) {
  std::ostringstream os;

  os << std::forward<T>(t);
  (void) std::initializer_list<int>{0, (void(os << std::forward<Ts>(ts)), 0)...};

  return os.str();
}

template<typename T, typename A, typename ...As>
inline void system_message(stateful_actor<T> *self, A &&a, As &&... as) {
  if (self->state.config->system_props.is_system_reporter_active)
    self->send(
        self->state.config->system_reporter, report::value, now(),
        str(std::forward<A>(a), std::forward<As>(as)...));
}

template<typename A, typename ...As>
inline void system_message(const scoped_actor &self,
                           const actor &system_reporter, A &&a, As &&... as) {
  self->send(system_reporter, report::value, now(),
             str(std::forward<A>(a), std::forward<As>(as)...));
}

template<typename T, typename ...As>
inline void generation_message(stateful_actor<T> *self, As &&... as) {
  if (self->state.config->system_props.is_generation_reporter_active)
    self->send(
        self->state.config->generation_reporter, std::forward<As>(as)...);
}

template<typename T, typename ...As>
inline void individual_message(stateful_actor<T> *self, As &&... as) {
  if (self->state.config->system_props.is_individual_reporter_active)
    self->send(
        self->state.config->individual_reporter, std::forward<As>(as)...);
}

template<typename Actor, typename Message>
inline void bus_message(Actor *self, Message &&msg) {
  self->state.config->bus.send(self, std::forward<Message>(msg));
}

template<typename A, typename ...As>
inline void log(A &&self, As &&... as) {
  aout(self) << str(std::forward<As>(as)...) << std::endl;
}

std::vector<actor> bind_remote_workers(actor_system &system, const std::vector<worker_node_info> &infos);

std::tuple<actor, actor, actor> bind_remote_reporters(actor_system &system, const reporter_node_info &info);
}
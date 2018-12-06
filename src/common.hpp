#pragma once

#include <chrono>
#include <type_traits>

// Commonly used data
namespace constants {
static constexpr const char POSSIBLE_VALUES_KEY[] =
    "possible_initialization_values";
static constexpr const char STABLE_REQUIRED_KEY[] = "stable_required";
static constexpr const char MINIMUM_AVERAGE_KEY[] = "minimum_average";

static constexpr const char* const ACTOR_PHASE_MAP[] = { "init_population",
    "execute_phase_1", "execute_phase_2", "execute_phase_3", "finish", "total" };

static const std::vector<std::string> SYSTEM_HEADERS { "Time", "Message" };

static const std::vector<std::string> TIME_HEADERS { "Start", "End",
    "Total (ms)", "Phase", "Generation", "Island" };

static const std::vector<std::string> INDIVIDUAL_HEADERS { "Generation",
    "Island", "Individual", "Fitness value" };
}

// Commonly used functions
const constexpr auto timeout = std::chrono::seconds(10);

inline auto now() noexcept {
  return std::chrono::high_resolution_clock::now();
}

template<typename E>
constexpr auto to_underlying(E e) noexcept
{
  return static_cast<std::underlying_type_t<E>>(e);
}

template<typename T, typename ... Ts>
auto str(T&& t, Ts&&... ts) {
  std::ostringstream os;

  os << std::forward<T>(t);
  using expander = int[];
  (void) expander { 0, (void(os << std::forward<Ts>(ts)),0)... };

  return os.str();
}

template<typename T, typename A, typename ...As>
inline void system_message(stateful_actor<T>* self, A&& a, As&&... as) {
  if (self->state.config->system_props.is_actor_reporter_active)
    self->send(self->state.config->system_reporter, report::value, now(),
               str(std::forward<A>(a), std::forward<As>(as)...));
}

template<typename A, typename ...As>
inline void system_message(const scoped_actor& self,
                           const actor& system_reporter, A&& a, As&&... as) {
  self->send(system_reporter, report::value, now(),
             str(std::forward<A>(a), std::forward<As>(as)...));
}

// aliases for common data structures
using island_id = std::size_t;

template<typename individual_value>
using sequence = std::vector<individual_value>;

template<typename individual, typename fitness_value>
using individual_wrapper = std::pair<individual, fitness_value>;

template<typename individual, typename fitness_value>
using individual_wrapper_pair = std::pair<individual_wrapper<individual, fitness_value>, individual_wrapper<individual, fitness_value>>;

template<typename individual, typename fitness_value>
using individual_collection = std::vector<individual_wrapper<individual, fitness_value>>;

template<typename individual, typename fitness_value>
using parent_collection = std::vector<individual_wrapper_pair<individual, fitness_value>>;

template<typename individual, typename fitness_value>
using migration_payload = std::vector<std::pair<island_id, individual_wrapper<individual, fitness_value>>>;

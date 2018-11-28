#pragma once

#include <chrono>
#include <type_traits>

const constexpr auto timeout = std::chrono::seconds(10);

template<typename E>
constexpr auto to_underlying(E e) noexcept
{
  return static_cast<std::underlying_type_t<E>>(e);
}

enum class actor_phase {
  init_population,
  execute_phase_1,
  execute_phase_2,
  execute_phase_3,
  finish,
  total
};

namespace constants {
static constexpr const char POSSIBLE_VALUES_KEY[] =
    "possible_initialization_values";
static constexpr const char STABLE_REQUIRED_KEY[] = "stable_required";
static constexpr const char MINIMUM_AVERAGE_KEY[] = "minimum_average";

static constexpr const char* const TIME_HEADERS_MIN[] = { "Start", "End",
    "Total (ms)" };

static constexpr const char* const ACTOR_PHASE_MAP[] = { "init_population",
    "execute_phase_1", "execute_phase_2", "execute_phase_3", "finish", "total" };

static inline auto now() {
  return std::chrono::high_resolution_clock::now();
}
}

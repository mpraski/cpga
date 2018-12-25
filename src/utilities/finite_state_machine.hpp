//
// Created by marcin on 22/12/18.
//

#ifndef GENETIC_ACTOR_FINITE_STATE_MACHINE_H
#define GENETIC_ACTOR_FINITE_STATE_MACHINE_H

#include <cstdint>
#include <tuple>
#include <functional>
#include <iostream>
#include <memory>

constexpr const auto SIZE = 2 << 7;
constexpr const auto MAX = SIZE - 1;

template<typename derived, typename state, typename event>
class state_machine {
 private:
  using state_u = typename std::conditional<std::is_enum<state>::value, std::underlying_type_t<state>, state>::type;
  using event_u = typename std::conditional<std::is_enum<event>::value, std::underlying_type_t<event>, event>::type;
  using transition_table = std::array<std::array<state, SIZE>, SIZE>;

  state_u current;
  transition_table trans;

  static constexpr transition_table prepare_table() {
    transition_table table{};
    for (uint8_t i = 0; i < SIZE; ++i)
      for (uint8_t j = 0; j < SIZE; ++j)
        table[i][j] = MAX;
    return table;
  }
 protected:
  using transition = std::tuple<event, state, state>;
  using transitions = std::initializer_list<transition>;

  constexpr state_machine(state current, transitions table = derived::DEFS)
      : current{static_cast<state_u>(current)}, trans{prepare_table()} {
    for (const auto&[e, s, n] : table) {
      trans[static_cast<event_u>(e)][static_cast<state_u>(s)] = n;
    }
  }
 public:
  inline auto in_state(state s) const noexcept {
    return static_cast<state>(current) == s;
  }

  void operator()(event e) {
    if (trans[static_cast<event_u>(e)][current] == MAX) {
      throw std::runtime_error("Unspecified transition");
    }
    current = trans[static_cast<event_u>(e)][current];
  }
};

#endif //GENETIC_ACTOR_FINITE_STATE_MACHINE_H

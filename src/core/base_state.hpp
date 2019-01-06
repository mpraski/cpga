//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_BASE_STATE_H
#define GENETIC_ACTOR_BASE_STATE_H

#include "data.hpp"

struct base_state {
  base_state() = default;
  explicit base_state(const shared_config &config);

  shared_config config;

  template<typename Actor>
  inline void join_group(Actor &&actor) const noexcept {
    config->bus.join(actor);
  }
};

#endif //GENETIC_ACTOR_BASE_STATE_H

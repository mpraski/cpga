//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_BASE_STATE_H
#define GENETIC_ACTOR_BASE_STATE_H

#include "data.hpp"

namespace cpga {
namespace core {
/**
 * @brief The base class for most other in core package.
 * @details Used to provide uniform access to essential configuration & reporting utilities.
 */
struct base_state {
  base_state() = default;
  explicit base_state(const shared_config &config);

  /**
   * @brief This shared_config member allows subclasses to access system, user & cluster configuration
   * as well as active reporter actors and the local message bus.
   */
  shared_config config;

  /**
   * Helper method for joining the message mus group using a CAF actor.
   * @tparam Actor
   * @param actor a CAF actor
   */
  template<typename Actor>
  inline void join_group(Actor &&actor) const noexcept {
    config->bus.join(actor);
  }
};
}
}

#endif //GENETIC_ACTOR_BASE_STATE_H

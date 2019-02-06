//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_DEFAULTS_H
#define GENETIC_ACTOR_DEFAULTS_H

#include "base_operator.hpp"

namespace cpga {
namespace core {
// Default implementations of optional operators (for default template arguments)
template<typename individual, typename fitness_value>
struct default_survival_selection_operator : base_operator {
  using base_operator::base_operator;

  void operator()(
      population<individual, fitness_value> &parents,
      population<individual, fitness_value> &offspring) const
  noexcept {
  }
};

template<typename individual, typename fitness_value>
struct default_elitism_operator : base_operator {
  using base_operator::base_operator;

  void operator()(
      population<individual, fitness_value> &main,
      population<individual, fitness_value> &elitists) const
  noexcept {
  }
};

template<typename individual, typename fitness_value>
struct default_migration_operator : base_operator {
  using base_operator::base_operator;

  migration_payload<individual, fitness_value> operator()(
      island_id from,
      population<individual, fitness_value> &population) const
  noexcept {
    return {};
  }
};

template<typename individual, typename fitness_value>
struct default_global_termination_check : base_operator {
  using base_operator::base_operator;

  bool operator()(
      const population<individual, fitness_value> &population) const
  noexcept {
    return false;
  }
};
}
}

#endif //GENETIC_ACTOR_DEFAULTS_H

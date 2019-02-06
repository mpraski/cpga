//
// Created by marcinpraski on 10/01/19.
//

#ifndef GENETIC_ACTOR_COMPONENTS_FAULT_DEFS_H
#define GENETIC_ACTOR_COMPONENTS_FAULT_DEFS_H

#include <common.hpp>
#include <iomanip>

namespace cpga {
namespace examples {
struct rbf_params {
  double c;
  double gamma;
};

std::ostream &operator<<(std::ostream &os, const rbf_params &params);

template<class Inspector>
typename Inspector::result_type inspect(Inspector &f, rbf_params &x) {
  return f(meta::type_name("rbf_params"), x.c, x.gamma);
}
}
}

#endif //GENETIC_ACTOR_COMPONENTS_FAULT_DEFS_H

//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_BASE_CLUSTER_STATE_H
#define GENETIC_ACTOR_BASE_CLUSTER_STATE_H

#include "../common.hpp"

namespace cpga {
namespace core {
struct base_cluster_state {
  base_cluster_state() = default;
  explicit base_cluster_state(const cluster_properties &cluster_props);

  cluster_properties cluster_props;
};
}
}

#endif //GENETIC_ACTOR_BASE_CLUSTER_STATE_H

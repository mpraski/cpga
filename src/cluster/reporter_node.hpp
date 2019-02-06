//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_REPORTER_NODE_H
#define GENETIC_ACTOR_REPORTER_NODE_H

#include <core.hpp>

namespace cpga {
using namespace utilities;
namespace cluster {
template<typename individual, typename fitness_value>
class reporter_node_driver : public base_cluster_driver {
 public:
  using base_cluster_driver::base_cluster_driver;

  void perform(scoped_actor &self) override {
    auto &system = self->system();
    auto &middleman = system.middleman();
    auto master_node = wait_for_master_node(middleman);
    auto port_factory = make_reporter_port_factory();

    reporter_node_info info{cluster_props.this_node_host};

    actor sys_rep;
    if (system_props.is_system_reporter_active) {
      if (system_props.system_reporter_log.empty()) {
        throw std::runtime_error("system_reporter_log is empty");
      }

      auto actor = self->spawn(system_reporter);
      auto port = port_factory();
      if (auto published{middleman.publish(actor, port)}; !published) {
        throw std::runtime_error(str("unable to publish system reporter: ", system.render(published.error())));
      } else {
        self->send(actor, init_reporter::value,
                   system_props.system_reporter_log, constants::SYSTEM_HEADERS);

        info.system_reporter_port = port;
        sys_rep = std::move(actor);
      }
    }

    if (system_props.is_generation_reporter_active) {
      if (system_props.generation_reporter_log.empty()) {
        throw std::runtime_error("actor_reporter_log is empty");
      }

      auto actor = system.spawn(time_reporter);
      auto port = port_factory();
      if (auto published{middleman.publish(actor, port)}; !published) {
        throw std::runtime_error(str("unable to publish system reporter: ", system.render(published.error())));
      } else {
        self->send(actor, init_reporter::value,
                   system_props.generation_reporter_log, constants::TIME_HEADERS);

        info.generation_reporter_port = port;
      }
    }

    if (system_props.is_individual_reporter_active) {
      if (system_props.individual_reporter_log.empty()) {
        throw std::runtime_error("actor_reporter_log is empty");
      }

      auto actor = system.spawn(individual_reporter < individual, fitness_value > );
      auto port = port_factory();
      if (auto published{middleman.publish(actor, port)}; !published) {
        throw std::runtime_error(str("unable to publish system reporter: ", system.render(published.error())));
      } else {
        self->send(actor, init_reporter::value,
                   system_props.individual_reporter_log,
                   constants::INDIVIDUAL_HEADERS);

        info.individual_reporter_port = port;
      }
    }

    if (system_props.is_system_reporter_active) {
      system_message(self, sys_rep, "Spawned reporters: ", info);
    }

    self->send(master_node, stage_discover_reporters::value, info);
    self->await_all_other_actors_done();
  }
};
}
}

#endif //GENETIC_ACTOR_REPORTER_NODE_H

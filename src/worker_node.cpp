//
// Created by marcin on 31/12/18.
//

#include <cpga/cluster/worker_node.hpp>

namespace cpga {
using namespace core;
using namespace atoms;
namespace cluster {
behavior worker_node_executor(stateful_actor<worker_node_executor_state> *self,
                              const system_properties &system_props,
                              const user_properties &user_props,
                              const cluster_properties &cluster_props,
                              const actor &master_node,
                              const group &message_bus_group,
                              const worker_spawner &factory,
                              const std::function<uint16_t()> &port_factory) {
  return {
      [=](const reporter_node_info &reporter_info) {
        auto &system = self->system();
        auto &middleman = system.middleman();

        auto[generation_reporter, individual_reporter, system_reporter] = bind_remote_reporters(system, reporter_info);
        auto config = make_shared_config(
            system_props,
            user_props,
            generation_reporter,
            individual_reporter,
            system_reporter,
            message_bus{message_bus_group}
        );

        self->state = worker_node_executor_state{config};

        auto workers = factory(self);

        self->set_down_handler([=](const down_msg &down) {
          if (std::any_of(std::begin(workers),
                          std::end(workers),
                          [src = down.source](const auto &worker) { return worker == src; })
              && ++self->state.workers_counter == workers.size()) {
            self->quit();
          } else if (down.source == master_node) {
            log(self, "Master node is down. Quitting...");
            self->quit();
          }
        });

        std::vector<uint16_t> ports;
        auto publish_worker = [&](const actor &worker) -> uint16_t {
          auto port = port_factory();
          if (auto published{middleman.publish(worker, port)}; !published) {
            throw std::runtime_error(str("unable to publish worker: ", system.render(published.error())));
          }
          system_message(self, "Publishing worker (actor id: ", worker.id(), ") on port ", port);
          return port;
        };
        std::transform(std::begin(workers), std::end(workers), std::back_inserter(ports), publish_worker);

        self->send(master_node,
                   stage_discover_workers::value,
                   worker_node_info{cluster_props.this_node_host, ports});
      }
  };
}

behavior worker_node(event_based_actor *self,
                     const actor &master_node,
                     const actor &executor,
                     const group &node_group) {
  self->join(node_group);
  self->monitor(master_node);

  return {
      [=](stage_initiate_worker_node) {
        self->send(master_node, stage_collect_workers::value);
      },
      [=](reporter_node_info &info) {
        self->send(executor, std::move(info));
      },
      [](const group_down_msg &) {}
  };
}

void worker_node_driver::perform(scoped_actor &self) {
  using namespace std::placeholders;

  auto &system = self->system();
  auto &middleman = system.middleman();
  auto master_node = wait_for_master_node(middleman);
  auto node_group = wait_for_node_group(system);
  auto message_bus_group = wait_for_message_bus_group(system);
  auto port_factory = make_worker_port_factory();

  auto executor =
      self->spawn<detached>(worker_node_executor,
                            system_props,
                            user_props,
                            cluster_props,
                            master_node,
                            message_bus_group,
                            std::bind(&worker_node_driver::spawn_workers, this, _1),
                            port_factory);

  auto node = self->spawn<detached>(worker_node, master_node, executor, node_group);

  anon_send(node, stage_initiate_worker_node::value);
  self->wait_for(executor);
  anon_send_exit(node, exit_reason::user_shutdown);
}
}
}
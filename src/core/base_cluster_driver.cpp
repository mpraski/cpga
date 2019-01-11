//
// Created by marcin on 31/12/18.
//

#include "base_cluster_driver.hpp"

std::function<uint16_t()> base_cluster_driver::make_reporter_port_factory() const {
  return create_counter(cluster_props.reporter_range_start);
}
std::function<uint16_t()> base_cluster_driver::make_worker_port_factory() const {
  return create_counter(cluster_props.worker_range_start);
}

actor base_cluster_driver::wait_for_master_node(io::middleman &m) const {
  auto &host = cluster_props.master_node_host;
  auto &port = cluster_props.master_node_port;

  return *poll([&] { return m.remote_actor(host, port); }, identity{});
}

group base_cluster_driver::wait_for_node_group(actor_system &s) const {
  auto &host = cluster_props.master_node_host;
  auto &port = cluster_props.master_group_port;

  return *poll([&] { return s.groups().get("remote", remote_group(constants::NODE_GROUP, host, port)); }, identity{});
}

group base_cluster_driver::wait_for_message_bus_group(actor_system &s) const {
  auto &host = cluster_props.master_node_host;
  auto &port = cluster_props.master_group_port;

  return *poll([&] { return s.groups().get("remote", remote_group(constants::MESSAGE_BUS_GROUP, host, port)); },
               identity{});
}

base_cluster_driver::base_cluster_driver(const system_properties &system_props,
                                         const user_properties &user_props,
                                         const cluster_properties &cluster_props) : system_props{system_props},
                                                                                    user_props{user_props},
                                                                                    cluster_props{cluster_props} {}

void base_cluster_driver::run(actor_system &system) {
  scoped_actor self{system};
  perform(self);
}
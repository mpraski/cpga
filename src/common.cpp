#include "common.hpp"

std::ostream &operator<<(std::ostream &os, const worker_node_info &x) {
  os << "worker_node_info(" << x.host << ", ";
  std::copy(std::begin(x.worker_ports), std::end(x.worker_ports), std::ostream_iterator<uint16_t>{os, ", "});
  return os << ")";
}

std::ostream &operator<<(std::ostream &os, const reporter_node_info &x) {
  return os << "reporter_node_info(" << x.host << ", " << x.system_reporter_port << ", " << x.generation_reporter_port
            << ", "
            << x.individual_reporter_port << ")";
}

std::vector<actor> bind_remote_workers(actor_system &system, const std::vector<worker_node_info> &infos) {
  std::vector<actor> workers;

  for (const auto &info : infos) {
    const auto &host = info.host;
    for (const auto &port : info.worker_ports) {
      if (auto actor{system.middleman().remote_actor(host, port)}; !actor) {
        throw std::runtime_error(str("Cannot connect to actor (", host, ":", port, ")"));
      } else {
        workers.emplace_back(std::move(*actor));
      }
    }
  }

  return workers;
}

std::tuple<actor, actor, actor> bind_remote_reporters(actor_system &system, const reporter_node_info &info) {
  const auto &host = info.host;
  auto generation_actor = system.middleman().remote_actor(host, info.generation_reporter_port);
  auto individual_actor = system.middleman().remote_actor(host, info.individual_reporter_port);
  auto system_actor = system.middleman().remote_actor(host, info.system_reporter_port);

  if (!generation_actor) throw std::runtime_error("Cannot connect to generation reporter");
  if (!individual_actor) throw std::runtime_error("Cannot connect to individual reporter");
  if (!system_actor) throw std::runtime_error("Cannot connect to system reporter");

  return std::make_tuple(*generation_actor, *individual_actor, *system_actor);
}
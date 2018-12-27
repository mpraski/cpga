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
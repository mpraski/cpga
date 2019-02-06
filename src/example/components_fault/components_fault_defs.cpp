//
// Created by marcinpraski on 10/01/19.
//

#include "components_fault_defs.hpp"

namespace cpga {
namespace examples {
std::ostream &operator<<(std::ostream &os, const rbf_params &params) {
  return os << std::fixed << std::setprecision(9) << "rbf_params(c=" << std::setfill(' ') << std::setw(15) << params.c
            << ", gamma=" << std::setfill(' ') << std::setw(10) << params.gamma << ")";
}
}
}
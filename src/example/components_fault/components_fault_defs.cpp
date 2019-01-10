//
// Created by marcinpraski on 10/01/19.
//

#include "components_fault_defs.hpp"

std::ostream &operator<<(std::ostream &os, const rbf_params &params) {
  return os << "rbf_params(c=" << params.c << ", gamma=" << params.gamma << ")";
}
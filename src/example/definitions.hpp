#pragma once

#include <fstream>
#include <vector>

std::ostream& operator<<(std::ostream &os, const std::vector<bool>& vec) {
  os << '[';
  for (const auto& b : vec)
    os << b << ',';
  os << ']';
  return os;
}

#pragma once

#include <fstream>
#include <vector>

std::ostream &operator<<(std::ostream &os, const std::vector<char> &vec) {
  os << '|';
  std::copy(std::begin(vec), std::end(vec), std::ostream_iterator<int>{os, "|"});
  return os;
}

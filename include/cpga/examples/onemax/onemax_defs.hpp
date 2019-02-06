#ifndef GENETIC_ACTOR_ONEMAX_DEFS_H
#define GENETIC_ACTOR_ONEMAX_DEFS_H

#include <fstream>
#include <vector>
#include <iterator>

std::ostream &operator<<(std::ostream &os, const std::vector<char> &vec) {
  os << '|';
  std::copy(std::begin(vec), std::end(vec), std::ostream_iterator<int>{os, "|"});
  return os;
}

#endif //GENETIC_ACTOR_ONEMAX_DEFS_H
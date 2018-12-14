#include "../core.hpp"

#include <vector>

class onemax_fitness_evaluation : public base_operator {
 public:
  using base_operator::base_operator;

  int operator()(const std::vector<bool>& ind) const noexcept {
    int i { 0 };
    for (const auto& b : ind)
      if (b) ++i;
    return i;
  }
};

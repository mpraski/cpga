#include "../core.hpp"

class onemax_fitness_evaluation : public base_operator {
 public:
  using base_operator::base_operator;

  inline int operator()(const std::vector<char> &ind) const noexcept {
    int i{0};
    for (auto b : ind) if (b) ++i;
    return i;
  }
};

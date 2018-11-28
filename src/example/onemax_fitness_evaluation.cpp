#include "../core.hpp"

#include <vector>

class onemax_fitness_evaluation : public base_state {
 public:
  onemax_fitness_evaluation() = default;

  onemax_fitness_evaluation(const shared_config& config)
      : base_state { config } {
  }

  int operator()(const std::vector<bool>& ind) const {
    int i { 0 };
    for (const auto& b : ind)
      if (b)
        ++i;
    return i;
  }
};

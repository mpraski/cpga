#include "../core.hpp"

#include <random>
#include <vector>

class bitstring_mutation : public base_state {
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution;
  std::function<double()> random;
 public:
  bitstring_mutation() = default;

  bitstring_mutation(const shared_config& config)
      : base_state { config },
        generator { config->system_props.crossover_seed },
        distribution { 0.0, 1.0 },
        random { std::bind(distribution, generator) } {
  }

  void operator()(individual_wrapper<std::vector<bool>, int>& wrapper) const {
    for (std::size_t i = 0; i < config->system_props.individual_size; ++i) {
      if (random() < config->system_props.mutation_probability) {
        wrapper.first[i].flip();
      }
    }
  }
};

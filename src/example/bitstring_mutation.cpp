#include "../core.hpp"

#include <random>
#include <vector>

class bitstring_mutation : public base_operator {
  using wrapper = individual_wrapper<std::vector<bool>, int>;
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution;
  std::function<double()> random_f;
 public:
  bitstring_mutation() = default;
  bitstring_mutation(const shared_config& config, island_id island_no)
      : base_operator { config, island_no },
        generator { config->system_props.crossover_seed },
        distribution { 0.0, 1.0 },
        random_f { std::bind(distribution, generator) } {
  }

  void operator()(wrapper& wrapper) const noexcept {
    for (std::size_t i = 0; i < config->system_props.individual_size; ++i) {
      if (random_f() < config->system_props.mutation_probability) {
        wrapper.first[i].flip();
      }
    }
  }
};

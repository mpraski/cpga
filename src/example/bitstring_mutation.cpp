#include "../core.hpp"

class bitstring_mutation : public base_operator {
  using wrapper = individual_wrapper<std::vector<char>, int>;
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution;
  std::function<double()> random_f;

  inline void flip(char &c) const noexcept {
    if (c == 0) c = 1; else c = 0;
  }
 public:
  bitstring_mutation() = default;
  bitstring_mutation(const shared_config &config, island_id island_no)
      : base_operator{config, island_no},
        generator{config->system_props.mutation_seed},
        distribution{0.0, 1.0},
        random_f{std::bind(distribution, generator)} {
  }

  void operator()(wrapper &wrapper) const noexcept {
    for (auto &c : wrapper.first) {
      if (random_f() < config->system_props.mutation_probability) {
        flip(c);
      }
    }
  }
};

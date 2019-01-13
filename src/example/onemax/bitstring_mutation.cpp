#include "bitstring_mutation.hpp"

bitstring_mutation::bitstring_mutation(const shared_config &config, island_id island_no)
    : base_operator{config, island_no},
      generator{config->system_props.mutation_seed},
      distribution{0.0, 1.0},
      random_f{std::bind(distribution, generator)} {
}

void bitstring_mutation::operator()(wrapper<sequence<char>, int> &wrapper) const noexcept {
  for (auto &c : wrapper.first) {
    if (random_f() < config->system_props.mutation_probability) {
      flip(c);
    }
  }
}
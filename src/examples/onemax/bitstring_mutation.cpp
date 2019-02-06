#include "bitstring_mutation.hpp"

namespace cpga {
using namespace core;
namespace examples {
bitstring_mutation::bitstring_mutation(const shared_config &config, island_id island_no)
    : base_operator{config, island_no},
      generator{config->system_props.mutation_seed},
      distribution{0.0, 1.0},
      random_f{std::bind(distribution, generator)} {
}

/**
 * @brief Perform mutation by looping though chars and flipping them if random_f()
 * is below defined mutation probability.
 * @param wrapper the bitstring individual and integer fitness value pair
 */
void bitstring_mutation::operator()(wrapper<sequence<char>, int> &wrapper) const noexcept {
  for (auto &c : wrapper.first) {
    if (random_f() < config->system_props.mutation_probability) {
      flip(c);
    }
  }
}
}
}
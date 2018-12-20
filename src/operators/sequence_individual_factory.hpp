#pragma once

#include <vector>

template<typename individual>
struct sequence_individual_vector_factory {
  inline auto operator()(size_t size) const {
    return std::vector<individual>(size);
  }
};
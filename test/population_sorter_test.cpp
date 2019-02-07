//
// Created by marcin on 07/02/19.
//

#include "catch2/catch.hpp"
#include <cpga/common.hpp>
#include <cpga/utilities/population_sorter.hpp>

TEST_CASE("population sorter sorts population in ascending order", "[population_sorter]") {
  const int sz{10};
  const auto comparator = [](const auto &a, const auto &b) {
    return a.second > b.second;
  };

  cpga::population<int, int> pop(sz);
  std::generate(std::begin(pop), std::end(pop), [c = 1]() mutable {
    auto p = std::make_pair(c, c * 2);
    ++c;
    return p;
  });

  REQUIRE(pop.size() == sz);

  SECTION("sorting population arranged in ascending order by fitness value") {
    cpga::utilities::population_sorter<int, int>::sort(pop);

    REQUIRE(pop.size() == 10);
    REQUIRE(pop[0].second == sz * 2);
    REQUIRE(std::is_sorted(std::begin(pop), std::end(pop), comparator));
  }
}
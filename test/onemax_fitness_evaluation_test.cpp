//
// Created by marcin on 11/02/19.
//

#include "catch2/catch.hpp"
#include "helpers/population_helper.hpp"
#include "helpers/shared_config_builder.hpp"
#include <cpga/examples/onemax/onemax_fitness_evaluation.hpp>

TEST_CASE("onemax_fitness_evaluation exhibits correct behaviour", "[onemax_fitness_evaluation]") {
  SECTION("with default settings") {
    cpga::wrapper<std::vector<char>, int> wrapper{{0, 1, 1, 1, 0, 1, 1}, 1};

    auto config = shared_config_builder(cpga::pga_model::GLOBAL).build();

    cpga::examples::onemax_fitness_evaluation evaluation{config, cpga::island_0};

    REQUIRE(evaluation(wrapper.first) == 5);
  }
}
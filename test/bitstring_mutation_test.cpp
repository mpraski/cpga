//
// Created by marcin on 11/02/19.
//

#include "catch2/catch.hpp"
#include "helpers/population_helper.hpp"
#include "helpers/shared_config_builder.hpp"
#include <cpga/examples/onemax/bitstring_mutation.hpp>

TEST_CASE("bitstring_mutation exhibits correct behaviour", "[bitstring_mutation]") {
  SECTION("when bitstring length is greater than 0") {
    size_t sz = 10;
    size_t ind_sz = 10;

    cpga::population<std::vector<char>, int> main{population_helper::sample_bitstring_population(sz, ind_sz)};

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withPopulationSize(sz)
        .withIndividualSize(ind_sz)
        .build();

    auto checker = [](const std::vector<char> &ind) {
      return std::all_of(std::begin(ind), std::end(ind), [](char c) {
        return c == 1 || c == 0;
      });
    };

    REQUIRE(checker(main[0].first));

    cpga::examples::bitstring_mutation mutation{config, cpga::island_0};
    mutation(main[0]);

    REQUIRE(checker(main[0].first));
  }

  SECTION("when bitstring length is equal to 0") {
    size_t sz = 10;
    size_t ind_sz = 0;

    cpga::population<std::vector<char>, int> main{population_helper::sample_bitstring_population(sz, ind_sz)};

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withPopulationSize(sz)
        .withIndividualSize(ind_sz)
        .build();

    auto checker = [](const std::vector<char> &ind) {
      return std::all_of(std::begin(ind), std::end(ind), [](char c) {
        return c == 1 || c == 0;
      });
    };

    REQUIRE(checker(main[0].first));

    cpga::examples::bitstring_mutation mutation{config, cpga::island_0};
    mutation(main[0]);

    REQUIRE(checker(main[0].first));
  }
}
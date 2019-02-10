//
// Created by marcin on 10/02/19.
//

#include "catch2/catch.hpp"
#include "helpers/population_helper.hpp"
#include "helpers/shared_config_builder.hpp"
#include <cpga/operators/best_individual_elitism.hpp>

TEST_CASE("best_individual_elitism exhibits correct behaviour", "[best_individual_elitism]") {
  SECTION("when there are less elitists than main population members") {
    size_t sz = 20;
    cpga::population<int, int> main{population_helper::sample_population(sz)};
    cpga::population<int, int> elitists{};

    cpga::population<int, int> expected{
        {20, 40},
        {19, 38},
        {18, 36},
    };

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withElitism(true)
        .withElitistsNumber(3)
        .build();

    cpga::operators::best_individual_elitism<int, int> elitism{config, cpga::island_0};

    elitism(main, elitists);

    REQUIRE(main.size() == sz - elitists.size());
    REQUIRE(!elitists.empty());
    REQUIRE(elitists == expected);
  }

  SECTION("when there are more elitists than main population members") {
    size_t sz = 2;
    cpga::population<int, int> main{population_helper::sample_population(sz)};
    cpga::population<int, int> elitists{};

    cpga::population<int, int> expected{
        {2, 4},
        {1, 2},
    };

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withElitism(true)
        .withElitistsNumber(3)
        .build();

    cpga::operators::best_individual_elitism<int, int> elitism{config, cpga::island_0};

    elitism(main, elitists);

    REQUIRE(main.empty());
    REQUIRE(!elitists.empty());
    REQUIRE(elitists == expected);
  }

  SECTION("when there 0 individuals are expected") {
    size_t sz = 10;
    cpga::population<int, int> main{population_helper::sample_population(sz)};
    cpga::population<int, int> elitists{};

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withElitism(true)
        .withElitistsNumber(0)
        .build();

    cpga::operators::best_individual_elitism<int, int> elitism{config, cpga::island_0};

    elitism(main, elitists);

    REQUIRE(main.size() == sz);
    REQUIRE(elitists.empty());
  }
}

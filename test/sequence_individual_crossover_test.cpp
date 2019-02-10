//
// Created by marcin on 10/02/19.
//

#include "catch2/catch.hpp"
#include "helpers/population_helper.hpp"
#include "helpers/shared_config_builder.hpp"
#include <cpga/operators/sequence_individual_crossover.hpp>

TEST_CASE("sequence_individual_crossover exhibits correct behaviour", "[sequence_individual_crossover]") {
  SECTION("with bool as the constituent type") {
    cpga::population<std::vector<bool>, int> main;
    std::vector<bool> constituents{true, false};
    cpga::wrapper_pair<std::vector<bool>, int> couple{
        {{true, true, false}, 0},
        {{false, true, true}, 1},
    };

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withPopulationSize(10)
        .withIndividualSize(3)
        .build();

    cpga::operators::sequence_individual_crossover<bool, int> crossover{config, cpga::island_0};

    REQUIRE(main.empty());

    crossover(std::back_inserter(main), couple);

    REQUIRE(main.size() == 2);
    REQUIRE(population_helper::sequence_population_in_range(main, constituents));
  }

  SECTION("with int as the constituent type") {
    cpga::population<std::vector<int>, int> main;
    std::vector<int> constituents{0, 2, 4, 6, 8};
    cpga::wrapper_pair<std::vector<int>, int> couple{
        {{0, 2, 4}, 0},
        {{4, 6, 8}, 1},
    };

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withPopulationSize(10)
        .withIndividualSize(3)
        .build();

    cpga::operators::sequence_individual_crossover<int, int> crossover{config, cpga::island_0};

    REQUIRE(main.empty());

    crossover(std::back_inserter(main), couple);

    REQUIRE(main.size() == 2);
    REQUIRE(population_helper::sequence_population_in_range(main, constituents));
  }

  SECTION("with std::string as the constituent type") {
    cpga::population<std::vector<std::string>, int> main;
    std::vector<std::string> constituents{"do", "re", "mi", "fa", "so", "la", "si", "dp"};
    cpga::wrapper_pair<std::vector<std::string>, int> couple{
        {{"do", "re", "mi"}, 0},
        {{"so", "la", "si"}, 1},
    };

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withPopulationSize(10)
        .withIndividualSize(3)
        .build();

    cpga::operators::sequence_individual_crossover<std::string, int> crossover{config, cpga::island_0};

    REQUIRE(main.empty());

    crossover(std::back_inserter(main), couple);

    REQUIRE(main.size() == 2);
    REQUIRE(population_helper::sequence_population_in_range(main, constituents));
  }
}
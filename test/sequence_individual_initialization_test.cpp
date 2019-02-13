//
// Created by marcin on 10/02/19.
//

#include "catch2/catch.hpp"
#include "helpers/population_helper.hpp"
#include "helpers/shared_config_builder.hpp"
#include <cpga/operators/sequence_individual_initialization.hpp>

TEST_CASE("sequence_individual_initialization exhibits correct behaviour", "[sequence_individual_initialization]") {
  SECTION("with bool as the constituent type") {
    cpga::population<std::vector<bool>, int> main;
    std::vector<bool> constituents{true, false};

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withPopulationSize(10)
        .withIndividualSize(10)
        .withUserProperty(cpga::strings::POSSIBLE_VALUES, constituents)
        .build();

    cpga::operators::sequence_individual_initialization<bool, int> initialization{config, cpga::island_0};

    REQUIRE(main.empty());

    initialization(std::back_inserter(main));

    REQUIRE(main.size() == config->system_props.population_size);
    REQUIRE(population_helper::sequence_population_in_range(main, constituents));
  }

  SECTION("with int as the constituent type") {
    cpga::population<std::vector<int>, int> main;
    std::vector<int> constituents{0, 2, 4, 8, 10, 1024};

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withPopulationSize(10)
        .withIndividualSize(10)
        .withUserProperty(cpga::strings::POSSIBLE_VALUES, constituents)
        .build();

    cpga::operators::sequence_individual_initialization<int, int> initialization{config, cpga::island_0};

    REQUIRE(main.empty());

    initialization(std::back_inserter(main));

    REQUIRE(main.size() == config->system_props.population_size);
    REQUIRE(population_helper::sequence_population_in_range(main, constituents));
  }

  SECTION("with std::string as the constituent type") {
    cpga::population<std::vector<std::string>, int> main;
    std::vector<std::string> constituents{"do", "re", "mi", "fa", "so", "la", "si", "dp"};

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withPopulationSize(10)
        .withIndividualSize(10)
        .withUserProperty(cpga::strings::POSSIBLE_VALUES, constituents)
        .build();

    cpga::operators::sequence_individual_initialization<std::string, int> initialization{config, cpga::island_0};

    REQUIRE(main.empty());

    initialization(std::back_inserter(main));

    REQUIRE(main.size() == config->system_props.population_size);
    REQUIRE(population_helper::sequence_population_in_range(main, constituents));
  }

  SECTION("with std::list as the collection type") {
    cpga::population<std::list<int>, int> main;
    std::vector<int> constituents{0, 2, 4, 8, 10, 1024};

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withPopulationSize(10)
        .withIndividualSize(10)
        .withUserProperty(cpga::strings::POSSIBLE_VALUES, constituents)
        .build();

    cpga::operators::sequence_individual_initialization<int, int, std::list<int>>
        initialization{config, cpga::island_0};

    REQUIRE(main.empty());

    initialization(std::back_inserter(main));

    REQUIRE(main.size() == config->system_props.population_size);
    REQUIRE(population_helper::sequence_population_in_range(main, constituents));
  }
}
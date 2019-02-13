//
// Created by marcin on 13/02/19.
//

#include "catch2/catch.hpp"
#include "helpers/population_helper.hpp"
#include "helpers/shared_config_builder.hpp"
#include <cpga/operators/average_fitness_global_termination_check.hpp>

TEST_CASE("average_fitness_global_termination_check exhibits correct behaviour",
          "[average_fitness_global_termination_check]") {
  SECTION("when population is empty") {
    size_t stable_required = 10;
    int average_fitness_value = 10;

    cpga::population<int, int> main{};

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withUserProperty(cpga::strings::STABLE_REQUIRED, stable_required)
        .withUserProperty(cpga::strings::MINIMUM_AVERAGE, average_fitness_value)
        .build();

    cpga::operators::average_fitness_global_termination_check<int, int> termination_check{config, cpga::island_0};

    REQUIRE(termination_check(main));
  }

  SECTION("when total fitness value is greater or equal to the minimum required") {
    size_t stable_required = 10;
    size_t sz = 20;
    int average_fitness_value = 10;

    cpga::population<int, int> main{population_helper::sample_population_with_fitness_value(sz, average_fitness_value)};

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withPopulationSize(sz)
        .withUserProperty(cpga::strings::STABLE_REQUIRED, stable_required)
        .withUserProperty(cpga::strings::MINIMUM_AVERAGE, average_fitness_value)
        .build();

    cpga::operators::average_fitness_global_termination_check<int, int> termination_check{config, cpga::island_0};

    SECTION("when enough stable measurements were taken") {
      bool should_stop = false;
      for (size_t i = 0; i < 10; ++i) {
        should_stop |= termination_check(main);
      }
      REQUIRE(should_stop);
    }

    SECTION("when not enough stable measurements were taken") {
      bool should_stop = false;
      for (size_t i = 0; i < 9; ++i) {
        should_stop |= termination_check(main);
      }
      REQUIRE(!should_stop);
    }
  }

  SECTION("when total fitness value is less than the minimum required") {
    size_t stable_required = 10;
    size_t sz = 20;
    int average_fitness_value = 10;

    cpga::population<int, int> main{population_helper::sample_population_with_fitness_value(sz, average_fitness_value - 1)};

    auto config = shared_config_builder(cpga::pga_model::GLOBAL)
        .withPopulationSize(sz)
        .withUserProperty(cpga::strings::STABLE_REQUIRED, stable_required)
        .withUserProperty(cpga::strings::MINIMUM_AVERAGE, average_fitness_value)
        .build();

    cpga::operators::average_fitness_global_termination_check<int, int> termination_check{config, cpga::island_0};

    SECTION("when enough stable measurements were taken") {
      bool should_stop = false;
      for (size_t i = 0; i < 20; ++i) {
        should_stop |= termination_check(main);
      }
      REQUIRE(!should_stop);
    }

    SECTION("when not enough stable measurements were taken") {
      bool should_stop = false;
      for (size_t i = 0; i < 9; ++i) {
        should_stop |= termination_check(main);
      }
      REQUIRE(!should_stop);
    }
  }
}
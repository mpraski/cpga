//
// Created by marcin on 10/02/19.
//

#include "catch2/catch.hpp"
#include "helpers/population_helper.hpp"
#include "helpers/shared_config_builder.hpp"
#include <cpga/operators/ring_best_migration.hpp>

TEST_CASE("ring_best_migration exhibits correct behaviour", "[ring_best_migration]") {
  SECTION("when population size is larger than migration quota") {
    size_t sz = 10;
    size_t islands_number = 3;
    cpga::island_id island_no = 2;
    cpga::population<int, int> main{population_helper::sample_population(sz)};
    cpga::migration_payload<int, int> expected{
        {(island_no + 1) % islands_number, {10, 20}},
        {(island_no + 1) % islands_number, {9, 18}},
        {(island_no + 1) % islands_number, {8, 16}},
    };

    auto config = shared_config_builder(cpga::pga_model::ISLAND)
        .withTotalPopulationSize(sz)
        .withIslandsNumber(islands_number)
        .withMigration(true)
        .withMigrationQuota(3)
        .build();

    cpga::operators::ring_best_migration<int, int> migration{config, island_no};

    auto payload = migration(island_no, main);

    REQUIRE(main.size() == sz - config->system_props.migration_quota);
    REQUIRE(payload.size() == config->system_props.migration_quota);
    REQUIRE(payload == expected);
    REQUIRE(std::all_of(std::begin(main), std::end(main), [&](const auto &wrapper) {
      return wrapper != expected[0].second
          && wrapper != expected[1].second
          && wrapper != expected[2].second;
    }));
  }

  SECTION("when population size is smaller than migration quota") {
    size_t sz = 10;
    size_t islands_number = 3;
    cpga::island_id island_no = 2;
    cpga::population<int, int> main{population_helper::sample_population(sz)};
    cpga::migration_payload<int, int> expected{
        {(island_no + 1) % islands_number, {10, 20}},
        {(island_no + 1) % islands_number, {9, 18}},
        {(island_no + 1) % islands_number, {8, 16}},
        {(island_no + 1) % islands_number, {7, 14}},
        {(island_no + 1) % islands_number, {6, 12}},
        {(island_no + 1) % islands_number, {5, 10}},
        {(island_no + 1) % islands_number, {4, 8}},
        {(island_no + 1) % islands_number, {3, 6}},
        {(island_no + 1) % islands_number, {2, 4}},
        {(island_no + 1) % islands_number, {1, 2}},
    };

    auto config = shared_config_builder(cpga::pga_model::ISLAND)
        .withTotalPopulationSize(sz)
        .withIslandsNumber(islands_number)
        .withMigration(true)
        .withMigrationQuota(12)
        .build();

    cpga::operators::ring_best_migration<int, int> migration{config, island_no};

    auto payload = migration(island_no, main);

    REQUIRE(main.empty());
    REQUIRE(payload.size() == sz);
    REQUIRE(payload == expected);
  }
}
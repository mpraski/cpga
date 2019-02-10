//
// Created by marcin on 10/02/19.
//

#include "shared_config_builder.hpp"

shared_config_builder::shared_config_builder(cpga::pga_model model) :
    system_props{},
    user_props{},
    generator{static_cast<unsigned long>(time(NULL))},
    distribution{0, 1000000ul},
    random_one{std::bind(distribution, generator)} {
  switch (model) {
    case cpga::pga_model::ISLAND:system_props.island_model();
      break;
    case cpga::pga_model::GLOBAL:system_props.global_model();
      break;
    case cpga::pga_model::GRID:system_props.grid_model();
      break;
    case cpga::pga_model::SEQUENTIAL:system_props.sequential_model();
      break;
  }

  system_props.initialization_seed = random_one();
  system_props.crossover_seed = random_one();
  system_props.mutation_seed = random_one();
  system_props.parent_selection_seed = random_one();
  system_props.survival_selection_seed = random_one();
  system_props.migration_seed = random_one();

  system_props.is_system_reporter_active = false;
  system_props.is_individual_reporter_active = false;
  system_props.is_generation_reporter_active = false;
}

shared_config_builder &shared_config_builder::withTotalPopulationSize(size_t population_size) {
  system_props.population_size = population_size;
  return *this;
}

shared_config_builder &shared_config_builder::withPopulationSize(size_t population_size) {
  system_props.population_size = population_size;
  return *this;
}

shared_config_builder &shared_config_builder::withIndividualSize(size_t individual_size) {
  system_props.individual_size = individual_size;
  return *this;
}

shared_config_builder &shared_config_builder::withIslandsNumber(size_t islands_number) {
  system_props.islands_number = islands_number;
  return *this;
}

shared_config_builder &shared_config_builder::withGenerationsNumber(size_t generations_number) {
  system_props.generations_number = generations_number;
  return *this;
}

shared_config_builder &shared_config_builder::withElitistsNumber(size_t elitists_number) {
  system_props.elitists_number = elitists_number;
  return *this;
}

shared_config_builder &shared_config_builder::withMigrationPeriod(size_t migration_period) {
  system_props.migration_period = migration_period;
  return *this;
}

shared_config_builder &shared_config_builder::withMigrationQuota(size_t migration_quota) {
  system_props.migration_quota = migration_quota;
  return *this;
}

shared_config_builder &shared_config_builder::withElitism(bool active) {
  system_props.is_elitism_active = active;
  return *this;
}

shared_config_builder &shared_config_builder::withSurvivalSelection(bool active) {
  system_props.is_survival_selection_active = active;
  return *this;
}

shared_config_builder &shared_config_builder::withMigration(bool active) {
  system_props.is_migration_active = active;
  return *this;
}

shared_config_builder &shared_config_builder::repeatingIndividualElements(bool active) {
  system_props.can_repeat_individual_elements = active;
  return *this;
}

shared_config_builder &shared_config_builder::addingIslandNosToSeed(bool active) {
  system_props.add_island_no_to_seed = active;
  return *this;
}

shared_config_builder &shared_config_builder::withCrossoverProbability(double probability) {
  system_props.crossover_probability = probability;
  return *this;
}

shared_config_builder &shared_config_builder::withMutationProbability(double probability) {
  system_props.mutation_probability = probability;
  return *this;
}

cpga::core::shared_config shared_config_builder::build() {
  system_props.compute_population_size();
  return cpga::core::make_shared_config(system_props, user_props, cpga::core::message_bus{});
}
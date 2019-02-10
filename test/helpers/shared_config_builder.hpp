//
// Created by marcin on 10/02/19.
//

#ifndef GENETIC_ACTOR_SHARED_CONFIG_BUILDER_H
#define GENETIC_ACTOR_SHARED_CONFIG_BUILDER_H

#include <cpga/core.hpp>
#include <cpga/common.hpp>

class shared_config_builder {
 private:
  cpga::core::system_properties system_props;
  cpga::core::user_properties user_props;

  std::default_random_engine generator;
  std::uniform_int_distribution<unsigned long> distribution;
  std::function<unsigned long()> random_one;
 public:
  shared_config_builder(cpga::pga_model model);

  shared_config_builder &withTotalPopulationSize(size_t population_size);
  shared_config_builder &withPopulationSize(size_t population_size);
  shared_config_builder &withIndividualSize(size_t individual_size);
  shared_config_builder &withIslandsNumber(size_t islands_number);
  shared_config_builder &withGenerationsNumber(size_t generations_number);
  shared_config_builder &withElitistsNumber(size_t elitists_number);
  shared_config_builder &withMigrationPeriod(size_t migration_period);
  shared_config_builder &withMigrationQuota(size_t migration_quota);
  shared_config_builder &withElitism(bool active);
  shared_config_builder &withSurvivalSelection(bool active);
  shared_config_builder &withMigration(bool active);
  shared_config_builder &repeatingIndividualElements(bool active);
  shared_config_builder &addingIslandNosToSeed(bool active);
  shared_config_builder &withCrossoverProbability(double probability);
  shared_config_builder &withMutationProbability(double probability);

  template<typename T>
  shared_config_builder &withUserProperty(std::string key, T &&prop) {
    user_props.emplace(key, std::forward<T>(prop));
    return *this;
  }

  cpga::core::shared_config build();
};

#endif //GENETIC_ACTOR_SHARED_CONFIG_BUILDER_H

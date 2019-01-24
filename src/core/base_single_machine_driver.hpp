//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_BASE_DRIVER_H
#define GENETIC_ACTOR_BASE_DRIVER_H

#include <atoms.hpp>
#include <common.hpp>
#include <utilities/reporter.hpp>
#include "data.hpp"

template<typename individual, typename fitness_value>
class base_single_machine_driver {
 private:
  std::shared_ptr<configuration> config;
  system_properties system_props;
  user_properties user_props;

  void start_reporters(std::shared_ptr<configuration> &config, actor_system &system,
                       scoped_actor &self) const {
    if (system_props.is_system_reporter_active) {
      if (system_props.system_reporter_log.empty()) {
        throw std::runtime_error("system_reporter_log is empty");
      }

      config->system_reporter = system.spawn(system_reporter);

      self->send(config->system_reporter, init_reporter::value,
                 system_props.system_reporter_log, constants::SYSTEM_HEADERS);

      system_message(self, config->system_reporter, "Spawning reporters");
    }

    if (system_props.is_generation_reporter_active) {
      if (system_props.generation_reporter_log.empty()) {
        throw std::runtime_error("actor_reporter_log is empty");
      }

      config->generation_reporter = system.spawn(time_reporter);

      self->send(config->generation_reporter, init_reporter::value,
                 system_props.generation_reporter_log, constants::TIME_HEADERS);
    }

    if (system_props.is_individual_reporter_active) {
      if (system_props.individual_reporter_log.empty()) {
        throw std::runtime_error("actor_reporter_log is empty");
      }

      config->individual_reporter = system.spawn(individual_reporter<individual, fitness_value>);

      self->send(config->individual_reporter, init_reporter::value,
                 system_props.individual_reporter_log,
                 constants::INDIVIDUAL_HEADERS);
    }
  }

  void stop_reporters(std::shared_ptr<configuration> &config, scoped_actor &self) const {
    if (system_props.is_system_reporter_active) {
      system_message(self, config->system_reporter, "Stopping reporters");

      self->send(config->system_reporter, exit_reporter::value);
    }

    if (system_props.is_generation_reporter_active) {
      self->send(config->generation_reporter, exit_reporter::value);
    }

    if (system_props.is_individual_reporter_active) {
      self->send(config->individual_reporter, exit_reporter::value);
    }
  }

  auto make_shared_configuration(actor_system &system,
                                 const system_properties &system_props,
                                 const user_properties &user_props) {
    auto message_bus_group = system.groups().get_local(constants::MESSAGE_BUS_GROUP);
    return std::make_shared<configuration>(system_props, user_props, message_bus{message_bus_group});
  }
 protected:
  virtual void perform(shared_config &config, scoped_actor &self) = 0;
 public:
  base_single_machine_driver(
      actor_system &system,
      const system_properties &system_props,
      const user_properties &user_props)
      : config{make_shared_configuration(system, system_props, user_props)},
        system_props{system_props},
        user_props{user_props} {
  }

  virtual ~base_single_machine_driver() = default;

  void run(actor_system &system) {
    scoped_actor self{system};

    shared_config const_config{std::const_pointer_cast<const configuration>(config)};

    show_model_info(self, system_props);
    start_reporters(config, system, self);
    perform(const_config, self);
    log(self, "** End of execution **");
    stop_reporters(config, self);
  }
};

#endif //GENETIC_ACTOR_BASE_DRIVER_H

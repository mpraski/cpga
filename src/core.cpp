#include "core.hpp"

configuration::configuration(const system_properties& system_props,
                             const user_properties& user_props)
    : system_props { system_props },
      user_props { user_props } {
}

base_state::base_state(const shared_config& config)
    : config { config } {
}

base_driver::base_driver(const system_properties& system_props,
                         const user_properties& user_props)
    : system_props { system_props },
      user_props { user_props } {

}

void base_driver::stop_reporters(configuration& conf,
                                 scoped_actor& self) const {
  if (system_props.is_actor_reporter_active) {
    self->send(conf.actor_reporter, exit_reporter::value);
  }

  if (system_props.is_individual_reporter_active) {
    self->send(conf.individual_reporter, exit_reporter::value);
  }
}

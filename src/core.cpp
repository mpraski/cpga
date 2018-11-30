#include "core.hpp"

configuration::configuration(const system_properties& system_props,
                             const user_properties& user_props)
    : system_props { system_props },
      user_props { user_props } {
}

base_state::base_state(const shared_config& config)
    : config { config } {
}

std::vector<std::string> base_driver::build_actor_reporter_headers() const {
  auto& time_headers = constants::TIME_HEADERS_MIN;

  std::vector<std::string> headers(std::begin(time_headers),
                                   std::end(time_headers));
  headers.push_back("Phase");
  headers.push_back("Generation");
  headers.push_back("Island");

  return headers;
}

std::vector<std::string> base_driver::build_individual_reporter_headers() const {
  auto& time_headers = constants::INDIVIDUAL_HEADERS_MIN;

  std::vector<std::string> headers(std::begin(time_headers),
                                   std::end(time_headers));

  return headers;
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

#include "core.hpp"

configuration::configuration(const system_properties& system_props,
                             const user_properties& user_props)
    : system_props { system_props },
      user_props { user_props } {
}

base_state::base_state(const shared_config& config)
    : config { config } {
}

base_operator::base_operator(const shared_config& config, island_id island_no)
    : base_state { config },
      island_no { island_no } {
}

//
// Created by marcin on 31/12/18.
//

#include "data.hpp"

configuration::configuration(const system_properties &system_props,
                             const user_properties &user_props,
                             actor &generation_reporter,
                             actor &individual_reporter,
                             actor &system_reporter,
                             message_bus &&bus) : system_props{system_props},
                                                 user_props{user_props},
                                                 generation_reporter{std::move(generation_reporter)},
                                                 individual_reporter{std::move(individual_reporter)},
                                                 system_reporter{std::move(system_reporter)},
                                                 bus{std::move(bus)} {}
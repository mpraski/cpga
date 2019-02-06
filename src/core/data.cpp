//
// Created by marcin on 31/12/18.
//

#include "data.hpp"

namespace cpga {
namespace core {
system_properties::system_properties() : total_population_size{0},
                                         population_size{0},
                                         islands_number{0},
                                         is_grid_model{false} {}

configuration::configuration(const system_properties &system_props,
                             const user_properties &user_props,
                             message_bus &&bus) : system_props{system_props},
                                                  user_props{user_props},
                                                  bus{std::move(bus)} {}

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
}
}
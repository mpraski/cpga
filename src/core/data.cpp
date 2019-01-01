//
// Created by marcin on 31/12/18.
//

#include "data.hpp"

configuration::configuration(const system_properties &system_props,
                             const user_properties &user_props) : system_props{system_props},
                                                                  user_props{user_props} {
}

configuration::configuration(const system_properties &system_props,
                             const user_properties &user_props,
                             const actor &generation_reporter,
                             const actor &individual_reporter,
                             const actor &system_reporter) : system_props{system_props},
                                                             user_props{user_props},
                                                             system_reporter{system_reporter},
                                                             generation_reporter{generation_reporter},
                                                             individual_reporter{individual_reporter} {
}
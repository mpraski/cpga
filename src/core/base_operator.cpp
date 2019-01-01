//
// Created by marcin on 31/12/18.
//

#include "base_operator.hpp"

base_operator::base_operator(const shared_config &config, island_id island_no) : base_state{config},
                                                                                 island_no{island_no} {
}

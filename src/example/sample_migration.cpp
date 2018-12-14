#include "../core.hpp"

#include <random>
#include <vector>

class sample_migration : public base_operator {
 public:
  using base_operator::base_operator;

  migration_payload<sequence<bool>, int> operator()(
      island_id from,
      individual_collection<sequence<bool>, int>& population) const {
    return {};
  }
};

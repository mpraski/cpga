#include "../core.hpp"

#include <random>
#include <vector>

class sample_migration : public base_state {
 public:
  sample_migration() = default;

  sample_migration(const shared_config& config)
      : base_state { config } {
  }

  migration_payload<sequence<bool>, int> operator()(
      island_id from,
      individual_collection<sequence<bool>, int>& population) const {
    return {};
  }
};

//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_BASE_CLUSTER_DRIVER_H
#define GENETIC_ACTOR_BASE_CLUSTER_DRIVER_H

#include <common.hpp>
#include "data.hpp"

class base_cluster_driver {
 private:
  template<typename T>
  static auto create_counter(T &&start) {
    return [c = std::forward<T>(start)]() mutable {
      return c++;
    };
  }

  static auto remote_group(const std::string &group, const std::string &host, uint16_t port) {
    return str(group, "@", host, ":", port);
  }

  template<typename Gen, typename Cond, typename Time = std::chrono::seconds>
  static auto poll(Gen &&generator,
                   Cond &&condition,
                   Time &&period = std::chrono::seconds(1),
                   size_t failed_attempts = 60) {
    auto checker = [&, c = size_t{}]() mutable {
      return std::make_pair(generator(), c++ != failed_attempts);
    };

    while (true) {
      if (auto[result, quota] = checker(); !condition(result)) {
        if (quota) {
          std::this_thread::sleep_for(period);
        } else {
          throw std::runtime_error(str("polling did not succeed within ", failed_attempts, " attempts"));
        }
      } else {
        return result;
      }
    }
  }
 protected:
  system_properties system_props;
  user_properties user_props;
  cluster_properties cluster_props;

  std::function<uint16_t()> make_reporter_port_factory() const;

  std::function<uint16_t()> make_worker_port_factory() const;

  actor wait_for_master_node(middleman &m) const;

  group wait_for_node_group(actor_system &s) const;

  group wait_for_message_bus_group(actor_system &s) const;

  virtual void perform(scoped_actor &self) = 0;
 public:
  base_cluster_driver(const system_properties &system_props,
                      const user_properties &user_props,
                      const cluster_properties &cluster_props);

  virtual ~base_cluster_driver() = default;

  void run(actor_system &system);
};

#endif //GENETIC_ACTOR_BASE_CLUSTER_DRIVER_H

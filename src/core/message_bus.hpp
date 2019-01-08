//
// Created by marcin on 03/01/19.
//

#ifndef GENETIC_ACTOR_MESSAGE_BUS_H
#define GENETIC_ACTOR_MESSAGE_BUS_H

#include <common.hpp>

class message_bus {
 private:
  group message_group;
 public:
  message_bus(const group &mg);

  template<typename Actor>
  void join(Actor &&actor) const noexcept {
    actor.join(message_group);
  }

  template<typename Actor>
  void send(Actor &&actor, std::string &&channel) const noexcept {
    send(std::forward<Actor>(actor), std::move(channel), "");
  }

  template<typename Actor>
  void send(Actor &&actor, std::string &&channel, std::string &&msg) const noexcept {
    actor.send(message_group, message_bus_receive::value, channel, msg);
  }
};

template<typename Fun>
static constexpr auto bus_receive(const std::string &key, Fun &&f) noexcept {
  return [key, fun = std::forward<Fun>(f)](message_bus_receive,
                                           const std::string &channel,
                                           const std::string &message) {
    if (channel == key) {
      fun(message);
    }
  };
}

#endif //GENETIC_ACTOR_MESSAGE_BUS_H

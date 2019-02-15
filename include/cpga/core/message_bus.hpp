//
// Created by marcin on 03/01/19.
//

#ifndef GENETIC_ACTOR_MESSAGE_BUS_H
#define GENETIC_ACTOR_MESSAGE_BUS_H

#include "../common.hpp"

namespace cpga {
namespace core {
/**
 * @brief A simplistic message bus for local communication between channels.
 * @details This class was introduces to reduce complexity of communication between dispatcher and the
 * workers. A CAF actor can register a lambda for receiving messaged sent to a given channel. Likewise,
 * the user can send a message (can be an empty string) to any channel.
 */
class message_bus {
 private:
  group message_group;
 public:
  message_bus() = default;
  message_bus(const group &mg);

  /**
   * Join the bus group.
   * @tparam Actor
   * @param actor
   */
  template<typename Actor>
  void join(Actor &&actor) const noexcept {
    actor.join(message_group);
  }

  /**
   * @brief Send an empty message to a given channel.
   * @tparam Actor
   * @param actor
   * @param channel
   */
  template<typename Actor>
  void send(Actor &&actor, std::string &&channel) const noexcept {
    send(std::forward<Actor>(actor), std::move(channel), "");
  }

  /**
   * @brief Send a message to a given channel.
   * @tparam Actor
   * @param actor
   * @param channel
   * @param msg
   */
  template<typename Actor>
  void send(Actor &&actor, std::string &&channel, std::string &&msg) const noexcept {
    actor->send(message_group, message_bus_receive::value, channel, msg);
  }
};

/**
 * @brief Register the listener.
 * @tparam Fun The callback executed when a message is sent to channel specified by key
 * @param key The channel this lambda will receive messages from
 * @param f The callback executed when a message is sent to channel specified by key
 * @return The augamened CAF message handler lambda executed when a message sent to 'key' channel is received
 */
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
}
}

#endif //GENETIC_ACTOR_MESSAGE_BUS_H

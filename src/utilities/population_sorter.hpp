//
// Created by marcin on 20/01/19.
//

#ifndef GENETIC_ACTOR_INDIVIDUAL_COMPARATOR_H
#define GENETIC_ACTOR_INDIVIDUAL_COMPARATOR_H

template<typename individual, typename fitness_value>
class population_sorter {
 private:
  constexpr static auto compFunctor = [](const auto &w1, const auto &w2) {
    return w1.second > w2.second;
  };
 public:
  inline static void sort(population<individual, fitness_value> &pop) {
    std::sort(std::begin(pop), std::end(pop), compFunctor);
  }
};

#endif //GENETIC_ACTOR_INDIVIDUAL_COMPARATOR_H

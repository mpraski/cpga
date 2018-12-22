//
// Created by marcin on 22/12/18.
//

#ifndef GENETIC_ACTOR_NODE_H
#define GENETIC_ACTOR_NODE_H

enum class node_type { MASTER, WORKER, REPORTER };

class node {
 private:
  node_type _type;
 public:
  node(node_type type);
  node_type type() const;
};

#endif //GENETIC_ACTOR_NODE_H

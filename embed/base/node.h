#pragma once

#include <stdbool.h>

#include "base/pubsub.h"


typedef struct {
  const char* name;
  bool active;
  bool (*init)(void);
  void (*term)(void);
} node_t;


#define NODE_REGISTER(name, init, term)                                       \
  node_t name = {#name, false, init, term}


extern bool node_init(node_t* node);
extern void node_term(node_t* node);

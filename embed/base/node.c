#include "base/node.h"

#include <assert.h>
#include <stdbool.h>

#include "base/logging.h"
#include "base/pubsub.h"


bool node_init(node_t* node) {
  assert(node && node->init);
  assert(!node->active);

  if (node->init()) {
    log_info("Initialization of %s is done.", node->name);
    node->active = true;
    return true;
  } else {
    log_error("Initialization of %s is failed.", node->name);
    return false;
  }
}


void node_term(node_t* node) {
  assert(node);
  assert(node->active);

  if (node->term) node->term();
  log_info("%s is terminated.", node->name);
}

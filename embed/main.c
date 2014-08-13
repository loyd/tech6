#include <assert.h>
#include <stdio.h>
#include <uv.h>

#include "base/config.h"
#include "base/logging.h"
#include "base/node.h"
#include "nodes/ahrs.h"

static node_t* nodes[] = {&ahrs};


static void terminate(int code) {
  for (int i = 0, len = sizeof(nodes)/sizeof(nodes[0]); i < len; ++i)
    node_term(nodes[i]);

  uv_stop(uv_default_loop());
  exit(code);
}


static void signal_handler(uv_signal_t* handle, int signum) {
  assert(handle);
  assert(signum == SIGINT);
  terminate(0);
}


int main(void) {
  cfg_init();

  // Initialize nodes.
  for (int i = 0, len = sizeof(nodes)/sizeof(nodes[0]); i < len; ++i)
    if (!node_init(nodes[i])) terminate(1);

  // Add listener to SIGINT.
  uv_signal_t sigint;
  uv_signal_init(uv_default_loop(), &sigint);
  uv_signal_start(&sigint, signal_handler, SIGINT);

  // Start loop.
  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

#pragma once

#include <stdbool.h>

#include "base/node.h"
#include "base/pubsub.h"


extern node_t ahrs;


/*
 * Event 'ahrs'
 */
extern event_t ev_ahrs;

typedef struct {
  float attitude[4];
} ev_ahrs_t;

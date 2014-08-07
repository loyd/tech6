#pragma once

#include <stdlib.h>

typedef void (*event_cb)(void* data);

typedef struct event_node_s {
  event_cb subscriber;
  struct event_node_s* next;
} event_t;

#define EVENT_INIT {NULL, NULL}

extern void publish(event_t* ev, void* data);
extern void subscribe(event_t* ev, event_cb cb);
extern void unsubscribe(event_t* ev, event_cb cb);
extern void unsubscribe_all(event_t* ev);

#define subscribe(ev, cb) subscribe(ev, (event_cb)cb)
#define unsubscribe(ev, cb) unsubscribe(ev, (event_cb)cb)

#include "base/pubsub.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "base/logging.h"

void publish(event_t* ev, void* data) {
  assert(ev && data);

  if (!ev->subscriber) return;
  for (; ev; ev = ev->next) {
    assert(ev->subscriber);
    ev->subscriber(data);
  }
}

void (subscribe)(event_t* ev, event_cb cb) {
  assert(ev && cb);

  if (ev->subscriber) {
    while (ev->next) ev = ev->next;
    ev = ev->next = malloc(sizeof(event_t));
  }

  ev->subscriber = cb;
  ev->next = NULL;
}

void (unsubscribe)(event_t* ev, event_cb cb) {
  assert(ev && cb);

  while (ev->subscriber == cb) {
    if (ev->next) {
      event_t* next = ev->next;
      ev->subscriber = next->subscriber;
      ev->next = next->next;
      free(next);
    } else {
      ev->subscriber = NULL;
    }
  }

  event_t* prev = ev;
  while ((ev = ev->next)) {
    assert(ev->subscriber);
    if (ev->subscriber == cb) {
      prev->next = ev->next;
      free(ev);
      ev = prev;
    } else {
      prev = ev;
    }
  }
}

void unsubscribe_all(event_t* ev) {
  assert(ev);
  event_t* next;

  while ((next = ev->next)) {
    ev->next = next->next;
    free(next);
  }

  ev->subscriber = NULL;
}

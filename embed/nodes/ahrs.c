#include "nodes/ahrs.h"

#include <assert.h>
#include <stdbool.h>
#include <uv.h>

#include "base/aux_math.h"
#include "base/logging.h"
#include "base/node.h"
#include "base/pubsub.h"
#include "control/madgwick_filter.h"
#include "devices/adxl345.h"
#include "devices/hmc5883l.h"
#include "devices/l3g4200d.h"


static int rate = 20;
static const char* bus = "/dev/i2c-1";


event_t ev_ahrs = EVENT_INIT;


static uv_timer_t timer_update;
static uint64_t last_run;


static adxl345_t* adxl345;
static hmc5883l_t* hmc5883l;
static l3g4200d_t* l3g4200d;
static madgwick_filter_t* filter;


static void term(void) {
  uv_timer_stop(&timer_update);
  if (filter) madgwick_filter_stop(filter);
  if (adxl345) adxl345_close(adxl345);
  if (hmc5883l) hmc5883l_close(hmc5883l);
  if (l3g4200d) l3g4200d_close(l3g4200d);
}


static void update(uv_timer_t* timer) {
  bool ok = adxl345_update(adxl345)
         && hmc5883l_update(hmc5883l)
         && l3g4200d_update(l3g4200d);

  if (!ok) {
    log_error("Failure while updating ahrs data. Stopped.");
    term();
    return;
  }

  uint64_t new_last_run = uv_hrtime();

  madgwick_filter_update(filter,
    deg_to_rad(l3g4200d->x), deg_to_rad(l3g4200d->y), deg_to_rad(l3g4200d->z),
    adxl345->x, adxl345->y, adxl345->z,
    hmc5883l->x, hmc5883l->y, hmc5883l->z,
    (new_last_run - last_run)/1e9f);

  last_run = new_last_run;
  publish(&ev_ahrs, filter->attitude);

  uv_update_time(uv_default_loop());
}


static bool init(void) {
  // It's necessary to initialize the timer before the termination.
  uv_timer_init(uv_default_loop(), &timer_update);
  adxl345 = NULL;
  hmc5883l = NULL;
  l3g4200d = NULL;
  filter = NULL;

  bool ok = (adxl345 = adxl345_open(bus, ADXL345_ADDR))
         && (hmc5883l = hmc5883l_open(bus, HMC5883L_ADDR))
         && (l3g4200d = l3g4200d_open(bus, L3G4200D_ADDR))
         && (filter = madgwick_filter_start(MADGWICK_FILTER_BETA))
         && adxl345_tune(adxl345, rate, 4.0f)
         && hmc5883l_tune(hmc5883l, rate, 4.0f)
         && l3g4200d_tune(l3g4200d, rate, 250.0f);

  if (!ok) goto failure;

  last_run = uv_hrtime();
  uv_timer_start(&timer_update, update, 1000/rate, 1000/rate);

  return true;

failure:
  term();
  return false;
}


NODE_REGISTER(ahrs, init, term);

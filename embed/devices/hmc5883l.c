#include "devices/hmc5883l.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "base/logging.h"
#include "devices/i2c.h"


const int8_t HMC5883L_ADDR = 0x1e;


static bool identify(i2c_dev_t* dev) {
  uint8_t check[3];
  if (!i2c_read(dev, 0x0a, check, 3)) return false;
  return check[0] == 'H' && check[1] == '4' && check[2] == '3';
}


hmc5883l_t* hmc5883l_open(const char* bus, int8_t addr) {
  assert(bus);

  i2c_dev_t* underline = i2c_open(bus, addr);
  if (!underline) return log_error("Cannot open hmc5883l.");

  if (!identify(underline))
    return i2c_close(underline)
      ? log_error("Device on %s:%#x doesn't hmc5883l.", bus, addr)
      : log_error("Cannot close hmc5883l on %s:%#x.", bus, addr);

  hmc5883l_t* dev = malloc(sizeof(hmc5883l_t));
  dev->underline = underline;
  dev->gain = NAN;

  return dev;
}


static const float rates[] = {.75, 1.5, 3, 7.5, 15, 30, 75};
static const float ranges[] = {.88, 1.3, 1.9, 2.5, 4, 4.7, 5.6, 8.1};


bool hmc5883l_tune(hmc5883l_t* dev, float rate, float range) {
  assert(dev);
  assert(rate > 0);
  assert(range > 0);

  // Setup rate.
  if (rate > 75) log_warning("Too high update rate for hmc5883l.");
  int rate_ctl = 0;
  while (rate_ctl < 6 && rates[rate_ctl] < rate) ++rate_ctl;
  rate = rates[rate_ctl];

  dev->buf[0] = 0x00;
  dev->buf[1] = rate_ctl << 2;

  if (!(i2c_write(dev->underline, dev->buf, 2)))
    return log_error("Cannot setup hmc5883l (rate = %f).", rate);

  // Setup range.
  if (range > 8.1f) log_warning("Too wide range for hmc5883l.");
  int range_ctl = 0;
  while (range_ctl < 7 && ranges[range_ctl] < range) ++range_ctl;
  range = ranges[range_ctl];
  dev->buf[0] = 0x01;
  dev->buf[1] = range_ctl << 5;

  if (!(i2c_write(dev->underline, dev->buf, 2)))
    return log_error("Cannot setup hmc5883l (range = %f).", range);

  dev->gain = range/2048 + 0.0003;

  // Start to measure.
  dev->buf[0] = 0x02;
  dev->buf[1] = 0x00;
  if (!(i2c_write(dev->underline, dev->buf, 2)))
    return log_error("Cannot start to measure of hmc5883l.");

  return true;
}


bool hmc5883l_update(hmc5883l_t* dev) {
  assert(dev);
  assert(!isnan(dev->gain));

  if (!i2c_read(dev->underline, 0x03, dev->buf, 6))
    return log_error("Cannot read data from hmc5883l.");

  dev->x = (int16_t)(dev->buf[0] << 8 | dev->buf[1]) * dev->gain;
  dev->y = (int16_t)(dev->buf[2] << 8 | dev->buf[3]) * dev->gain;
  dev->z = (int16_t)(dev->buf[4] << 8 | dev->buf[5]) * dev->gain;

  return true;
}


bool hmc5883l_close(hmc5883l_t* dev) {
  assert(dev);
  bool res = true;

  dev->buf[0] = 0x02;
  dev->buf[1] = 0x02;
  if (!i2c_write(dev->underline, dev->buf, 2))
    res = log_error("Cannot stop hmc5883l.");

  if (!i2c_close(dev->underline))
    res = log_error("Cannot close hmc5883l.");

  free(dev);
  return res;
}

#include "devices/adxl345.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "base/logging.h"
#include "devices/i2c.h"


const int8_t ADXL345_ADDR = 0x53;


static bool identify(i2c_dev_t* dev) {
  uint8_t check;
  return i2c_read(dev, 0x00, &check, 1) && check == 0xe5;
}


adxl345_t* adxl345_open(const char* bus, int8_t addr) {
  assert(bus);

  i2c_dev_t* underline = i2c_open(bus, addr);
  if (!underline) return log_error("Cannot open adxl345.");

  if (!identify(underline))
    return i2c_close(underline)
      ? log_error("Device on %s:%#x doesn't adxl345.", bus, addr)
      : log_error("Cannot close adxl345 on %s:%#x.", bus, addr);

  adxl345_t* dev = malloc(sizeof(adxl345_t));
  dev->underline = underline;
  dev->gain = NAN;

  return dev;
}


static const float rates[] = {.10, .20, .39, .78, 1.56, 3.13, 6.25,
                              12.5, 25, 50, 100, 200, 400, 800, 1600, 3200};
                          // |<~~~~  low power mode ~~~~>|

static const float ranges[] = {2, 4, 8, 16};


bool adxl345_tune(adxl345_t* dev, float rate, float range) {
  assert(dev);
  assert(rate > 0);
  assert(range > 0);

  // Setup rate
  if (rate > 3200) log_warning("Too high update rate for adxl345.");
  int rate_ctl = 0;
  while (rate_ctl < 15 && rates[rate_ctl] < rate) ++rate_ctl;
  rate = rates[rate_ctl];
  if (12.5 <= rate && rate <= 400) rate_ctl |= 0x10;  // low power mode

  dev->buf[0] = 0x2c;
  dev->buf[1] = rate_ctl;

  if (!(i2c_write(dev->underline, dev->buf, 2)))
    return log_error("Cannot setup adxl345 (rate = %f).", rate);

  // Setup range
  if (range > 16) log_warning("Too wide range for adxl345.");
  int range_ctl = 0;
  while (range_ctl < 3 && ranges[range_ctl] < range) ++range_ctl;
  range = ranges[range_ctl];
  dev->buf[0] = 0x31;
  dev->buf[1] = range_ctl | 0x08;  // full resolution mode

  if (!(i2c_write(dev->underline, dev->buf, 2)))
    return log_error("Cannot setup adxl345 (range = %f).", range);

  dev->gain = range/(512 << range_ctl);

  // Start to measure
  dev->buf[0] = 0x2d;
  dev->buf[1] = 0x08;
  if (!(i2c_write(dev->underline, dev->buf, 2)))
    return log_error("Cannot start to measure of adxl345.");

  return true;
}


bool adxl345_update(adxl345_t* dev) {
  assert(dev);
  assert(!isnan(dev->gain));

  if (!i2c_read(dev->underline, 0x32, dev->buf, 6))
    return log_error("Cannot read data from adxl345.");

  dev->x = (int16_t)(dev->buf[1] << 8 | dev->buf[0]) * dev->gain;
  dev->y = (int16_t)(dev->buf[3] << 8 | dev->buf[2]) * dev->gain;
  dev->z = (int16_t)(dev->buf[5] << 8 | dev->buf[4]) * dev->gain;

  return true;
}


bool adxl345_close(adxl345_t* dev) {
  assert(dev);
  bool res = true;

  dev->buf[0] = 0x2d;
  dev->buf[1] = 0x00;
  if (!i2c_write(dev->underline, dev->buf, 2))
    res = log_error("Cannot stop adxl345.");

  if (!i2c_close(dev->underline))
    res = log_error("Cannot close adxl345.");

  free(dev);
  return res;
}

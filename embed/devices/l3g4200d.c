#include "devices/l3g4200d.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <tgmath.h>

#include "base/logging.h"
#include "devices/i2c.h"


const int8_t L3G4200D_ADDR = 0x69;


static bool identify(i2c_dev_t* dev) {
  uint8_t check;
  return i2c_read(dev, 0x0f, &check, 1) && check == 0xd3;
}


l3g4200d_t* l3g4200d_open(const char* bus, int8_t addr) {
  assert(bus);

  i2c_dev_t* underline = i2c_open(bus, addr);
  if (!underline)
    return log_error("Cannot open l3g4200d on %s:%#x.", bus, addr);

  if (!identify(underline))
    return i2c_close(underline)
      ? log_error("Device on %s:%#x doesn't l3g4200d.", bus, addr)
      : log_error("Cannot close l3g4200d on %s:%#x.", bus, addr);

  l3g4200d_t* dev = malloc(sizeof(l3g4200d_t));
  dev->underline = underline;
  dev->gain = NAN;

  return dev;
}


bool l3g4200d_tune(l3g4200d_t* dev, float rate, float range) {
  assert(dev);
  assert(rate > 0);
  assert(range > 0);

  // Setup rate
  if (rate > 800) log_warning("Too high update rate for l3g4200d.");
  dev->buf[0] = 0x20;
  dev->buf[1] = rate <= 100 ? (rate = 100, 0x2f)
              : rate <= 200 ? (rate = 200, 0x6f)
              : rate <= 400 ? (rate = 400, 0xaf)
              : (rate = 800, 0xef);

  if (!(i2c_write(dev->underline, dev->buf, 2)))
    return log_error("Cannot setup l3g4200d (rate = %f).", rate);

  // Setup range
  if (range > 2000) log_warning("Too wide range for l3g4200d.");
  dev->buf[0] = 0x23;
  dev->buf[1] = range <= 250 ? (range = 250, 0x00)
              : range <= 500 ? (range = 500, 0x10)
              : (range = 2000, 0x20);

  if (!(i2c_write(dev->underline, dev->buf, 2)))
    return log_error("Cannot setup l3g4200d (range = %f).", range);

  dev->gain = range/32768;
  return true;
}


bool l3g4200d_update(l3g4200d_t* dev) {
  assert(dev);
  assert(!isnan(dev->gain));

  if (!i2c_read(dev->underline, 0x80 | 0x28, dev->buf, 6))
    return log_error("Cannot read data from l3g4200d.");

  dev->x = (int16_t)(dev->buf[1] << 8 | dev->buf[0]) * dev->gain;
  dev->y = (int16_t)(dev->buf[3] << 8 | dev->buf[2]) * dev->gain;
  dev->z = (int16_t)(dev->buf[5] << 8 | dev->buf[4]) * dev->gain;

  return true;
}


bool l3g4200d_close(l3g4200d_t* dev) {
  assert(dev);
  bool res = true;

  dev->buf[0] = 0x20;
  dev->buf[1] = 0x00;
  if (!i2c_write(dev->underline, dev->buf, 2))
    res = log_error("Cannot stop l3g4200d.");

  if (!i2c_close(dev->underline))
    res = log_error("Cannot close l3g4200d.");

  free(dev);
  return res;
}

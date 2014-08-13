#include "devices/bmp085.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <tgmath.h>

#include "base/logging.h"
#include "devices/i2c.h"


const int8_t BMP085_ADDR = 0x77;


static bool read_calibration(bmp085_t* dev) {
  assert(dev);

  uint8_t buf[22];
  if (!i2c_read(dev->underline, 0xaa, buf, 22))
    return false;

  dev->ac1 = buf[0]  << 8 | buf[1];
  dev->ac2 = buf[2]  << 8 | buf[3];
  dev->ac3 = buf[4]  << 8 | buf[5];
  dev->ac4 = buf[6]  << 8 | buf[7];
  dev->ac5 = buf[8]  << 8 | buf[9];
  dev->ac6 = buf[10] << 8 | buf[11];
  dev->b1  = buf[12] << 8 | buf[13];
  dev->b2  = buf[14] << 8 | buf[15];
  dev->mb  = buf[16] << 8 | buf[17];
  dev->mc  = buf[18] << 8 | buf[19];
  dev->md  = buf[20] << 8 | buf[21];

  return true;
}


bmp085_t* bmp085_open(const char* bus, int8_t addr) {
  assert(bus);

  i2c_dev_t* underline = i2c_open(bus, addr);
  if (!underline)
    return log_error("Cannot open bmp085 on %s:%#x.", bus, addr);

  bmp085_t* dev = malloc(sizeof(bmp085_t));
  dev->underline = underline;
  dev->oss = -1;

  if (!read_calibration(dev)) {
    log_error("Cannot read calibration data from bmp085 on %s:%#x.", bus, addr);
    bmp085_close(dev);
    return NULL;
  }

  return dev;
}


static bool request_ut(bmp085_t* dev) {
  assert(dev);

  dev->buf[0] = 0xf4;
  dev->buf[1] = 0x2e;
  if (!i2c_write(dev->underline, dev->buf, 2))
    return log_error("Failure while requesting temperature from bmp085.");

  return true;
}


static bool request_up(bmp085_t* dev) {
  assert(dev);

  dev->buf[0] = 0xf4;
  dev->buf[1] = 0x34 + (dev->oss << 6);
  if (!i2c_write(dev->underline, dev->buf, 2))
    return log_error("Failure while requesting pressure from bmp085.");

  return true;
}


bool bmp085_tune(bmp085_t* dev, float rate) {
  assert(dev);
  assert(rate > 0);

  // Setup rate.
  if (rate > 208)
    log_warning("Too high update rate (%f) for bmp085 (%s:%#x).",
                rate, dev->underline->bus, dev->underline->addr);

  dev->oss = (rate <= 128) + (rate <= 72) + (rate <= 39);
  dev->temp_idle = round(rate) - 1;
  dev->temp_count = 0;

  return request_ut(dev);
}


static bool update_temperature(bmp085_t* dev) {
  assert(dev);

  if (!i2c_read(dev->underline, 0xf6, dev->buf, 2))
    return false;

  int32_t ut = dev->buf[0] << 8 | dev->buf[1];
  int32_t x1 = ((ut - dev->ac6) * dev->ac5) >> 15;
  int32_t x2 = (dev->mc << 11)/(x1 + dev->md);

  dev->b5 = x1 + x2;
  dev->temperature = ((dev->b5 + 8) >> 4) * 0.1f;

  return true;
}


static bool update_pressure(bmp085_t* dev) {
  assert(dev);

  if (!i2c_read(dev->underline, 0xf6, dev->buf, 3))
    return false;

  int32_t up, x1, x2, x3, b3, b6, p;
  uint32_t b4, b7;

  up = (dev->buf[0] << 16 | dev->buf[1] << 8 | dev->buf[2]) >> (8-dev->oss);
  b6 = dev->b5 - 4000;

  x1 = (dev->b2 * (b6*b6 >> 12)) >> 11;
  x2 = (dev->ac2 * b6) >> 11;
  x3 = x1 + x2;
  b3 = (((dev->ac1*4 + x3) << dev->oss) + 2) >> 2;

  x1 = (dev->ac3 * b6) >> 13;
  x2 = (dev->b1 * (b6*b6 >> 12)) >> 16;
  x3 = (x1 + x2 + 2) >> 2;
  b4 = (dev->ac4 * (uint32_t)(x3 + 32768)) >> 15;

  b7 = ((uint32_t)up - b3) * (50000 >> dev->oss);
  p = b7 < 0x80000000 ? (b7 << 1)/b4 : (b7/b4) << 1;

  x1 = (p >> 8) * (p >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  p += (x1 + x2 + 3791) >> 4;
  dev->pressure = p;

  return true;
}


bool bmp085_update(bmp085_t* dev) {
  assert(dev);
  assert(dev->oss > -1);

  if (dev->temp_count == 0) {
    if (!(update_temperature(dev) && request_up(dev))) goto error;
    ++dev->temp_count;
  } else if (dev->temp_count == dev->temp_idle) {
    if (!(update_pressure(dev) && request_ut(dev))) goto error;
    dev->temp_count = 0;
  } else {
    if (!(update_pressure(dev) && request_up(dev))) goto error;
    ++dev->temp_count;
  }

  return true;

error:
  return log_error("Failure while updating of bmp085.");
}


bool bmp085_close(bmp085_t* dev) {
  assert(dev);

  bool ok = i2c_close(dev->underline);
  if (!ok) log_error("Cannot close bmp085.");

  free(dev);
  return ok;
}

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "devices/i2c.h"


typedef struct {
  i2c_dev_t* underline;
  float temperature;
  int32_t pressure;

  uint8_t temp_count;
  uint8_t temp_idle;
  int8_t oss;

  int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
  uint16_t ac4, ac5, ac6;
  int32_t b5;

  uint8_t buf[3];
} bmp085_t;


extern const int8_t BMP085_ADDR;

extern bmp085_t* bmp085_open(const char* bus, int8_t addr);
extern bool bmp085_tune(bmp085_t* dev, float rate);
extern bool bmp085_update(bmp085_t* dev);
extern bool bmp085_close(bmp085_t* dev);

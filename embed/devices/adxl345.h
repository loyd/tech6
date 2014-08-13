#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "devices/i2c.h"


typedef struct {
  i2c_dev_t* underline;
  float gain;
  float x, y, z;
  uint8_t buf[6];
} adxl345_t;


extern const int8_t ADXL345_ADDR;

extern adxl345_t* adxl345_open(const char* bus, int8_t addr);
extern bool adxl345_tune(adxl345_t* dev, float rate, float range);
extern bool adxl345_update(adxl345_t* dev);
extern bool adxl345_close(adxl345_t* dev);

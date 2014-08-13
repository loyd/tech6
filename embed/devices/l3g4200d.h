#pragma once

#include <stdbool.h>

#include "devices/i2c.h"


typedef struct {
  i2c_dev_t* underline;
  float gain;
  float x, y, z;
  uint8_t buf[6];
} l3g4200d_t;


extern const int8_t L3G4200D_ADDR;

extern l3g4200d_t* l3g4200d_open(const char* bus, int8_t addr);
extern bool l3g4200d_tune(l3g4200d_t* dev, float rate, float range);
extern bool l3g4200d_update(l3g4200d_t* dev);
extern bool l3g4200d_close(l3g4200d_t* dev);

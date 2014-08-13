#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "devices/i2c.h"


typedef struct {
  i2c_dev_t* underline;
  float gain;
  float x, y, z;
  uint8_t buf[6];
} hmc5883l_t;


extern const int8_t HMC5883L_ADDR;

extern hmc5883l_t* hmc5883l_open(const char* bus, int8_t addr);
extern bool hmc5883l_tune(hmc5883l_t* dev, float rate, float range);
extern bool hmc5883l_update(hmc5883l_t* dev);
extern bool hmc5883l_close(hmc5883l_t* dev);

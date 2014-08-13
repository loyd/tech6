#pragma once

#include <stdbool.h>
#include <stdint.h>


typedef struct {
  char* bus;
  int8_t addr;
  int fd;
} i2c_dev_t;


extern i2c_dev_t* i2c_open(const char* bus, int8_t addr);
extern bool i2c_write(i2c_dev_t* dev, void* buf, uint8_t size);
extern bool i2c_read(i2c_dev_t* dev, uint8_t reg, void* buf, uint8_t size);
extern bool i2c_close(i2c_dev_t* dev);

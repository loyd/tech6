#include "devices/i2c.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "base/logging.h"


i2c_dev_t* i2c_open(const char* bus, int8_t addr) {
  assert(bus);
  assert(1 < addr >> 2 && addr >> 2 < 0x1e);

  int fd = open(bus, O_RDWR);
  if (fd < 0)
    return log_error("Cannot open %s:%#x: %s.", bus, addr, strerror(errno));

  if (ioctl(fd, I2C_SLAVE, addr) < 0)
    return log_error("Cannot setup %s:%#x as slave: %s.",
                     bus, addr, strerror(errno));

  i2c_dev_t* dev = malloc(sizeof(i2c_dev_t));
  dev->bus = strdup(bus);
  dev->addr = addr;
  dev->fd = fd;

  return dev;
}


bool i2c_write(i2c_dev_t* dev, void* buf, uint8_t size) {
  assert(dev && buf);
  assert(size > 0);

  return (write(dev->fd, buf, size) == (int)size) ||
    log_error("Cannot write to %s:%#x: %s.",
              dev->bus, dev->addr, strerror(errno));
}


bool i2c_read(i2c_dev_t* dev, uint8_t reg, void* buf, uint8_t size) {
  assert(dev && buf);
  assert(size > 0);

  if (!(write(dev->fd, &reg, 1) == 1 && read(dev->fd, buf, size) == (int)size))
    return log_error("Cannot read from %s:%#x: %s.",
                     dev->bus, dev->addr, strerror(errno));

  return true;
}


bool i2c_close(i2c_dev_t* dev) {
  assert(dev);

  bool res = close(dev->fd) == 0;
  if (!res) log_error("Cannot close %s:%#x: %s.",
                      dev->bus, dev->addr, strerror(errno));

  free(dev->bus);
  free(dev);

  return res;
}

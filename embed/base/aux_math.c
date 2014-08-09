#include "base/aux_math.h"

#include <assert.h>
#include <stdint.h>
#include <tgmath.h>


float inv_sqrt(float x) {
  float halfx = 0.5f * x;
  float y = x;
  int32_t i = *(int32_t*)&y;
  i = 0x5f3759df - (i>>1);
  y = *(float*)&i;
  y = y * (1.5f - (halfx * y * y));
  return y;
}


float press_to_alt(int32_t p) {
  assert(p > 0);
  return 44330 * (1 - pow(p/101325.0f, 0.19029496f));
}


void quat_to_euler(float q[4], float* yaw, float* pitch, float* roll) {
  assert(yaw && pitch && roll);
  *yaw = atan2(2*q[1]*q[2]-2*q[0]*q[3], 2*q[0]*q[0] + 2*q[1]*q[1]-1);
  *pitch = -asin(2*q[1]*q[3] + 2*q[0]*q[2]);
  *roll = atan2(2*q[2]*q[3]-2*q[0]*q[1], 2*q[0]*q[0] + 2*q[3]*q[3]-1);
}

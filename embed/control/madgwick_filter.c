#include "control/madgwick_filter.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "base/aux_math.h"
#include "base/logging.h"


const float MADGWICK_FILTER_BETA = 0.1f;


madgwick_filter_t* madgwick_filter_start(float beta) {
  assert(0 <= beta && beta <= 1);
  madgwick_filter_t* filter = malloc(sizeof(madgwick_filter_t));
  filter->beta = beta;
  filter->attitude[0] = 1.0f;
  filter->attitude[1] = filter->attitude[2] = filter->attitude[3] = 0.0f;

  return filter;
}


void madgwick_filter_update(madgwick_filter_t* filter,
                            float gx, float gy, float gz,
                            float ax, float ay, float az,
                            float mx, float my, float mz,
                            float dt) {
  float beta = filter->beta;
  float q0 = filter->attitude[0];
  float q1 = filter->attitude[1];
  float q2 = filter->attitude[2];
  float q3 = filter->attitude[3];

  float recip_norm;
  float s0, s1, s2, s3;
  float qdot1, qdot2, qdot3, qdot4;
  float hx, hy;
  float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2bz, _4bx, _4bz,
        _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3,
        q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

  // Rate of change of quaternion from gyroscope.
  qdot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
  qdot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
  qdot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
  qdot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

  // Compute feedback only if accelerometer measurement valid.
  if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
    // Normalize accelerometer measurement.
    recip_norm = inv_sqrt(ax*ax + ay*ay + az*az);
    ax *= recip_norm;
    ay *= recip_norm;
    az *= recip_norm;

    // Normalize magnetometer measurement.
    recip_norm = inv_sqrt(mx*mx + my*my + mz*mz);
    mx *= recip_norm;
    my *= recip_norm;
    mz *= recip_norm;

    // Auxiliary variables to avoid repeated arithmetic.
    _2q0mx = 2.0f * q0 * mx;
    _2q0my = 2.0f * q0 * my;
    _2q0mz = 2.0f * q0 * mz;
    _2q1mx = 2.0f * q1 * mx;
    _2q0 = 2.0f * q0;
    _2q1 = 2.0f * q1;
    _2q2 = 2.0f * q2;
    _2q3 = 2.0f * q3;
    _2q0q2 = 2.0f * q0 * q2;
    _2q2q3 = 2.0f * q2 * q3;
    q0q0 = q0 * q0;
    q0q1 = q0 * q1;
    q0q2 = q0 * q2;
    q0q3 = q0 * q3;
    q1q1 = q1 * q1;
    q1q2 = q1 * q2;
    q1q3 = q1 * q3;
    q2q2 = q2 * q2;
    q2q3 = q2 * q3;
    q3q3 = q3 * q3;

    // Reference direction of Earth's magnetic field.
    hx = mx * q0q0 - _2q0my * q3 + _2q0mz * q2 + mx * q1q1 + _2q1 * my * q2
       + _2q1 * mz * q3 - mx * q2q2 - mx * q3q3;
    hy = _2q0mx * q3 + my * q0q0 - _2q0mz * q1 + _2q1mx * q2 - my * q1q1
       + my * q2q2 + _2q2 * mz * q3 - my * q3q3;
    _2bx = sqrt(hx * hx + hy * hy);
    _2bz = -_2q0mx * q2 + _2q0my * q1 + mz * q0q0 + _2q1mx * q3 - mz * q1q1
         + _2q2 * my * q3 - mz * q2q2 + mz * q3q3;
    _4bx = 2.0f * _2bx;
    _4bz = 2.0f * _2bz;

    // Gradient decent algorithm corrective step.
    s0 = -_2q2 * (2.0f*q1q3 - _2q0q2 - ax) + _2q1 * (2.0f*q0q1 + _2q2q3 - ay)
       - _2bz * q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx)
       + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3)
       - my) + _2bx * q2 * (_2bx *(q0q2+q1q3) + _2bz*(0.5f - q1q1 - q2q2) - mz);
    s1 = _2q3 * (2.0f*q1q3 - _2q0q2 - ax) + _2q0 * (2.0f*q0q1 + _2q2q3 - ay)
       - 4.0f * q1 * (1 - 2.0f*q1q1 - 2.0f*q2q2 - az) + _2bz * q3 * (_2bx
       * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q2 + _2bz
       * q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q3
       - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
    s2 = -_2q0 * (2.0f*q1q3 - _2q0q2 - ax) + _2q3 * (2.0f*q0q1 + _2q2q3 - ay)
       - 4.0f * q2 * (1 - 2.0f*q1q1 - 2.0f*q2q2 - az) + (-_4bx * q2 - _2bz * q0)
       * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q1
       + _2bz * q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx
       * q0 - _4bz*q2) * (_2bx * (q0q2+q1q3) + _2bz * (0.5f - q1q1-q2q2) - mz);
    s3 = _2q1 * (2.0f*q1q3 - _2q0q2 - ax) + _2q2 * (2.0f*q0q1 + _2q2q3 - ay)
       + (-_4bx * q3 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3
       - q0q2) - mx) + (-_2bx * q0 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2bz
       * (q0q1 + q2q3) - my) + _2bx * q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f
       - q1q1 - q2q2) - mz);

    recip_norm = inv_sqrt(s0*s0 + s1*s1 + s2*s2 + s3*s3);
    s0 *= recip_norm;
    s1 *= recip_norm;
    s2 *= recip_norm;
    s3 *= recip_norm;

    // Apply feedback step.
    qdot1 -= beta * s0;
    qdot2 -= beta * s1;
    qdot3 -= beta * s2;
    qdot4 -= beta * s3;
  }

  // Integrate rate of change of quaternion to yield quaternion.
  q0 += qdot1 * dt;
  q1 += qdot2 * dt;
  q2 += qdot3 * dt;
  q3 += qdot4 * dt;

  // Normalize quaternion.
  recip_norm = inv_sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
  filter->attitude[0] = q0 * recip_norm;
  filter->attitude[1] = q1 * recip_norm;
  filter->attitude[2] = q2 * recip_norm;
  filter->attitude[3] = q3 * recip_norm;
}


void madgwick_filter_stop(madgwick_filter_t* filter) {
  free(filter);
}

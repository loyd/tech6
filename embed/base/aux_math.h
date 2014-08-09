#pragma once

#include <stdint.h>
#include <tgmath.h>


/*!
 * Calculate inverse square root.
 * @see http://en.wikipedia.org/wiki/Fast_inverse_square_root
 */
extern float inv_sqrt(float x);

/*! Convert pressure to altitude for air. */
extern float press_to_alt(int32_t p);

/*!
 * Convert quaternion to Euler's angles.
 * @param q               quaternion
 * @param yaw,pitch,roll  angles
 */
extern void quat_to_euler(float q[4], float* yaw, float* pitch, float* roll);


#define deg_to_rad(x) (x) * M_PI/180
#define rad_to_deg(x) (x) * 180*M_1_PI

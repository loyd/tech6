#pragma once


typedef struct {
  float attitude[4];  //!< The тormalized quaternion of sensor frame.
  float beta;         //!< Twice proportional gain.
} madgwick_filter_t;


/*! The default value of `beta`. */
extern const float MADGWICK_FILTER_BETA;

extern madgwick_filter_t* madgwick_filter_start(float beta);

/*!
 * Update current state using measurements of sensors and delta of time.
 * Optimized for minimal arithmetic:
 *   + 75    - 85    * 190    / 4    √ 5
 * @param filter   current data
 * @param gx,gy,gz gyroscope data [rad/s]
 * @param ax,ay,az accelerometer data [g]
 * @param mx,my,mz magnetometer data [T] or [G]
 * @param dt       time since the last update [s]
 */
extern void madgwick_filter_update(madgwick_filter_t* filter,
                                   float gx, float gy, float gz,
                                   float ax, float ay, float az,
                                   float mx, float my, float mz,
                                   float dt);

extern void madgwick_filter_stop(madgwick_filter_t* filter);

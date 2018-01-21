/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Copyright (C) 2015 Motorola Mobility LLC
 */

#ifndef ANDROID_SENSORS_H
#define ANDROID_SENSORS_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <linux/input.h>
#include <linux/limits.h>

#include <hardware/hardware.h>
#include <hardware/sensors.h>
#include "mot_sensorhub_stml0xx.h"

__BEGIN_DECLS

/*****************************************************************************/

typedef enum sensor_handle {
	MIN_SENSOR_ID,
	ID_A,              /* Accelerometer */
#ifdef _ENABLE_GYROSCOPE
	ID_G,              /* Gyroscope */
	ID_UNCALIB_GYRO,   /* Uncalibrated Gyroscope */
	ID_GAME_RV,        /* Game Rotation Vector */
	ID_GRAVITY,        /* Gravity */
	ID_LA,             /* Linear Acceleration */
#endif //_ENABLE_GYROSCOPE
	ID_L,	           /* Light */
	ID_P,	           /* Proximity */
	ID_DR,	           /* Display Rotate */
	ID_FU,	           /* Flat Up */
	ID_FD,	           /* Flat Down */
	ID_S,	           /* Stowed */
	ID_CA,	           /* Camera Activate Gesture */
	ID_A2,	           /* Secondary Accel */
#ifdef _ENABLE_MAGNETOMETER
	ID_M,	           /* Magnetometer */
	ID_UM,	           /* Uncalibrated Magnetometer */
	ID_OR,	           /* Orientation */
        ID_GEOMAG_RV,      /* GeoMagnetic Rotation Vector */
        ID_RV,             /* Rotation Vector */
#endif
#ifdef _ENABLE_CHOPCHOP
	ID_CC,             /* Chopchop gesture */
#endif
#ifdef _ENABLE_LIFT
	ID_LF, 	           /* Lift Gesture */
#endif
#ifdef _ENABLE_REARPROX
	ID_RP, 	           /* Moto Rear Proximity */
#endif
#ifdef _ENABLE_REARPROX_2
	ID_RP_2, 	   /* Moto Rear Proximity2*/
#endif
#ifdef _ENABLE_CAPSENSE
	ID_CS, 	           /* Moto CapSense */
#endif
#ifdef _ENABLE_PEDO
	ID_STEP_DETECTOR, /* Step detector */
	ID_STEP_COUNTER, /* Step counter */
#endif
	ID_GLANCE_GESTURE, /* Glance Gesture */
	ID_MOTO_GLANCE_GESTURE, /* Moto Glance Gesture */
        ID_MOTION_DETECT,
        ID_STATIONARY_DETECT,
	MAX_SENSOR_ID
} sensor_handle_t;

/*****************************************************************************/
/* Sensor Hub definitions */

#ifdef _ENABLE_KXCJ9
/* KXCJ9 configured to +/-8G 14-bit mode
 * 14-bit Register Data range -8192 ~ +8191
 * 1024 LSG = 1G
*/
#define RANGE_G                     (8.0f)
#define LSG                         (1024.0f)
#endif

#ifdef _ENABLE_BMI160
#define RANGE_G                     (16.0f)
#define LSG                         (2048.0f)
/* conversion of angular velocity(millidegrees/second) to rad/s */
#define CONVERT_G                   ((2000.0f/32767.0f) * ((float)(M_PI/180.0f)))
#define CONVERT_G_P                 (CONVERT_G)
#define CONVERT_G_R                 (CONVERT_G)
#define CONVERT_G_Y                 (CONVERT_G)
#define CONVERT_BIAS_G_P            (CONVERT_G)
#define CONVERT_BIAS_G_R            (CONVERT_G)
#define CONVERT_BIAS_G_Y            (CONVERT_G)
#endif

/* Conversion of acceleration data to SI units (m/s^2) */
#define CONVERT_A                   (GRAVITY_EARTH / LSG)
#define CONVERT_A_X                 (CONVERT_A)
#define CONVERT_A_Y                 (CONVERT_A)
#define CONVERT_A_Z                 (CONVERT_A)

/* Proximity uncovered and covered values */
#define PROX_UNCOVERED              (100.0f)
#define PROX_COVERED                (3.0f)
#define PROX_SATURATED              (1.0f)

/* Display rotate values */
#define DISP_FLAT                   (0x10)
#define DISP_UNKNOWN                (-1.0f)

/* Flat up / down values */
#define FLAT_NOTDETECTED            (0.0f)
#define FLAT_DETECTED               (1.0f)

/*****************************************************************************/

#ifdef _ENABLE_MAGNETOMETER

// conversion of magnetic data to uT units
#define CONVERT_M                   (1.0f/16.0f)
#define CONVERT_M_X                 (CONVERT_M)
#define CONVERT_M_Y                 (CONVERT_M)
#define CONVERT_M_Z                 (CONVERT_M)
#define CONVERT_BIAS_M_X            (CONVERT_M)
#define CONVERT_BIAS_M_Y            (CONVERT_M)
#define CONVERT_BIAS_M_Z            (CONVERT_M)
#define CONVERT_O                   (1.0f/64.0f)
#define CONVERT_O_Y                 (CONVERT_O)
#define CONVERT_O_P                 (CONVERT_O)
#define CONVERT_O_R                 (CONVERT_O)

/* conversion of acceleration data to SI units (m/s^2) */
/* 720 LSB = 1G */
#define AKSC_LSG					(720.0f)
#define CONVERT_A_AKM					CONVERT_AKM_G

/* conversion of orientation data (Q6) to degree units */
#define CONVERT_OR					(1.0f / 64.0f)
/* conversion of rotation vector (Q14) data to float */
#define CONVERT_RV					CONVERT_Q14

#define SENSOR_STATE_MASK           (0x7FFF)

#endif /* _ENABLE_MAGNETOMETER */

/*****************************************************************************/

__END_DECLS

#endif  // ANDROID_SENSORS_H

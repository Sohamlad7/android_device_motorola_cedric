/*
 * Copyright (C) 2009-2015 Motorola Mobility
 *
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

#ifndef ANDROID_SENSORS_H
#define ANDROID_SENSORS_H

#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <type_traits>

#include <linux/input.h>
#include <linux/motosh.h>

#include <hardware/hardware.h>
#include <hardware/sensors.h>

__BEGIN_DECLS

static inline int motosh_ioctl (int fd, int ioctl_number, ...) {
    va_list ap;
    void * arg;
    int status = 0;
    int error = 0;

    va_start(ap, ioctl_number);
    arg = va_arg(ap, void *);
    va_end(ap);

    do {
        status = ioctl(fd, ioctl_number, arg);
        error = errno;
    } while ((status != 0) && (error == -EINTR));
    return status;
}

/*****************************************************************************/

/*
 * The SENSORS Module
 */

/*****************************************************************************/

// 2048 LSG = 1G
#define LSG                         (2048.0f)

// conversion of acceleration data to SI units (m/s^2)
#define CONVERT_A                   (GRAVITY_EARTH / LSG)
#define CONVERT_A_X                 (CONVERT_A)
#define CONVERT_A_Y                 (CONVERT_A)
#define CONVERT_A_Z                 (CONVERT_A)

// conversion of linear accel data
#define CONVERT_A_LIN               (CONVERT_A)

// conversion of gravity data
#define CONVERT_GRAVITY             (GRAVITY_EARTH/((float)INT16_MAX))

// conversion of Quaternion data
#define CONVERT_QUA                 (1.0f/16384.0f)

// conversion of magnetic data to uT units
#define CONVERT_M                   (1.0f/8.0f)
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

// proximity uncovered and covered values
#define PROX_UNCOVERED              (100.0f)
#define PROX_COVERED                (3.0f)
#define PROX_SATURATED              (1.0f)

// flat up / down values
#define FLAT_NOTDETECTED            (0.0f)
#define FLAT_DETECTED               (1.0f)

// Display rotate values
#define DISP_FLAT                   0x10
#define DISP_UNKNOWN                (-1.0f)

// conversion of angular velocity(millidegrees/second) to rad/s
#define CONVERT_G                   ((2000.0f/32767.0f) * ((float)(M_PI/180.0f)))
#define CONVERT_G_P                 (CONVERT_G)
#define CONVERT_G_R                 (CONVERT_G)
#define CONVERT_G_Y                 (CONVERT_G)
#define CONVERT_BIAS_G_P            (CONVERT_G)
#define CONVERT_BIAS_G_R            (CONVERT_G)
#define CONVERT_BIAS_G_Y            (CONVERT_G)

#define CONVERT_RV                  (1.0f/32767.f)
#define CONVERT_B                   (1.0f/100.0f)

#define CONVERT_T                   (1.0f/10.0f)

#define SENSOR_STATE_MASK           (0x7FFF)

/*****************************************************************************/

__END_DECLS

#endif  // ANDROID_SENSORS_H

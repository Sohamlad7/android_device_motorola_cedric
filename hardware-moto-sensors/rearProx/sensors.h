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

#ifndef ANDROID_SENSORS_H
#define ANDROID_SENSORS_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <linux/input.h>

#include <hardware/hardware.h>
#include <hardware/sensors.h>


__BEGIN_DECLS

/*****************************************************************************/

#define LOG_TAG "NativeSensors"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define SENSORS_PROXIMITY_HANDLE		4

#define MAX_SENSORS		(20)
#define SYSFS_MAXLEN		(50)
#define SYSFS_CLASS		"/sys/class/sensors/"
#define SYSFS_NAME		"name"
#define SYSFS_VENDOR		"vendor"
#define SYSFS_VERSION		"version"
#define SYSFS_HANDLE		"handle"
#define SYSFS_TYPE		"type"
#define SYSFS_MAXRANGE		"max_range"
#define SYSFS_RESOLUTION	"resolution"
#define SYSFS_POWER		"sensor_power"
#define SYSFS_MINDELAY		"min_delay"
#define SYSFS_ENABLE		"enable"
#define SYSFS_POLL_DELAY	"poll_delay"
#define SYSFS_CAL_PARAMS	"cal_params"
#define SYSFS_FIFORESVCNT	"fifo_reserved_event_count"
#define SYSFS_FIFOMAXCNT	"fifo_max_event_count"
#define SYSFS_MAXLATENCY	"max_latency"
#define SYSFS_MAXDELAY		"max_delay"
#define SYSFS_FLUSH		"flush"
#define SYSFS_FLAGS		"flags"


#define PROXIMITY_NAME		"proximity"
#define CAPSENSOR_NAME		"capsensor"


/* The hardware sensor type supported by HAL */
//#define SUPPORTED_SENSORS_TYPE	( \
//		(1ULL << SENSOR_TYPE_PROXIMITY))
static inline bool is_sensor_supported(int type)
{
	switch (type) {
		default:
			return true;
		}
}


/*****************************************************************************/
/* Helper function to convert sensor type to common sensor name */
static inline const char* type_to_name(int type)
{
	switch (type) {
		default:
			return "proximity";
	}
}

__END_DECLS

#endif  // ANDROID_SENSORS_H

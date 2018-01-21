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

#include <float.h>
#include <string.h>

#include <cutils/log.h>

#include <hardware/sensors.h>
#include "mot_sensorhub_stml0xx.h"

#include "SensorList.h"

#include "Sensors.h"
#include "SensorsPollContext.h"


static int poll__close(struct hw_device_t *dev)
{
	(void)dev;
        return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev,
		int handle, int enabled) {
	SensorsPollContext *ctx = (SensorsPollContext *)dev;
	return ctx->activate(handle, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t *dev,
		int handle, int64_t ns) {
	SensorsPollContext *ctx = (SensorsPollContext *)dev;
	return ctx->setDelay(handle, ns);
}

/*!
 * \brief Implement Android HAL poll()
 *
 * From [source.android.com](https://source.android.com/devices/sensors/hal-interface.html)
 *
 * Returns an array of sensor data by filling the data argument. This function
 * must block until events are available. It will return the number of events
 * read on success, or a negative error number in case of an error.
 *
 * The number of events returned in data must be less or equal to the count
 * argument. This function shall never return 0 (no event).
 *
 * \param[in]  dev   the device to poll
 * \param[out] data  the returned data items
 * \param[in]  count the maximum number of returned data items
 *
 * \returns negative on failure, the number of returned data items on success,
 *          and never 0.
 */
static int poll__poll(struct sensors_poll_device_t *dev,
		sensors_event_t* data, int count) {
	SensorsPollContext *ctx = (SensorsPollContext *)dev;
	int ret = 0;
	do {
		ret = ctx->pollEvents(data, count);
	} while( ret == 0 );
	return ret;
}

static int poll__batch(sensors_poll_device_1_t *dev,
		int handle, int flags, int64_t ns, int64_t timeout) {
	SensorsPollContext *ctx = (SensorsPollContext *)dev;
	return ctx->batch(handle, flags, ns, timeout);
}

static int poll__flush(sensors_poll_device_1_t *dev,
		int handle) {
	SensorsPollContext *ctx = (SensorsPollContext *)dev;
	return ctx->flush(handle);
}

/** Open a new instance of a sensor device using name */
static int open_sensors(const struct hw_module_t* module, const char* id,
			struct hw_device_t** device)
{
	(void)id;
	int status = -EINVAL;

	SensorsPollContext *dev = SensorsPollContext::getInstance();
	if (dev) {
		memset(&dev->device, 0, sizeof(sensors_poll_device_1_t));

		dev->device.common.tag      = HARDWARE_DEVICE_TAG;
		dev->device.common.version  = SENSORS_DEVICE_API_VERSION_1_3;
		dev->device.common.module   = const_cast<hw_module_t*>(module);
		dev->device.common.close    = poll__close;
		dev->device.activate        = poll__activate;
		dev->device.setDelay        = poll__setDelay;
		dev->device.poll            = poll__poll;
		dev->device.batch           = poll__batch;
		dev->device.flush           = poll__flush;

		*device = &dev->device.common;
		status = 0;
	} else {
		ALOGE("out of memory: new failed for SensorsPollContext");
	}

	return status;
}

static int sensors__get_sensors_list(struct sensors_module_t* module,
				     struct sensor_t const** list) 
{
	(void)module;
	*list = &sSensorList[0];
	return sSensorList.size();
}

static int sensors__set_operation_mode(unsigned int mode)
{
	// We only support normal operation. No loopback mode.
	return mode == 0 ? 0 : -EINVAL;
}

static struct hw_module_methods_t sensors_module_methods = {
	open: open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
	common: {
		tag: HARDWARE_MODULE_TAG,
		version_major: 1,
		version_minor: 0,
		id: SENSORS_HARDWARE_MODULE_ID,
		name: "Motorola Sensors Module",
		author: "Motorola",
		methods: &sensors_module_methods,
		dso: NULL,
		reserved: {0},
	},
	get_sensors_list: sensors__get_sensors_list,
	set_operation_mode: sensors__set_operation_mode
};

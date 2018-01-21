/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Not a Contribution.
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

//#define LOG_TAG "Sensors"

#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <linux/input.h>
#include <utils/Atomic.h>
#include <utils/Log.h>

#include "sensors.h"
#include "ProximitySensor.h"
#include "NativeSensorManager.h"
#include "sensors_extension.h"
/*****************************************************************************/

static int open_sensors(const struct hw_module_t* module, const char* id,
						struct hw_device_t** device);

static int sensors__get_sensors_list(struct sensors_module_t*,
								 struct sensor_t const** list)
{
	NativeSensorManager& sm(NativeSensorManager::getInstance());

	return sm.getSensorList(list);
}

static struct hw_module_methods_t sensors_module_methods = {
		.open = open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
		.common = {
				.tag = HARDWARE_MODULE_TAG,
				.module_api_version = 1,
				.hal_api_version = 0,
				.id = SENSORS_HARDWARE_MODULE_ID,
				.name = "Quic Sensor module",
				.author = "Quic",
				.methods = &sensors_module_methods,
				.dso = NULL,
				.reserved = {0},
		},
		.get_sensors_list = sensors__get_sensors_list,
};

struct sensors_poll_context_t {
	// extension for sensors_poll_device_1, must be first
	struct sensors_poll_device_1_ext_t device;// must be first
	sensors_poll_context_t();
	~sensors_poll_context_t();
	int activate(int handle, int enabled);
	int setDelay(int handle, int64_t ns);
	int pollEvents(sensors_event_t* data, int count);
	int batch(int handle, int sample_ns, int latency_ns);
	int flush(int handle);

private:
	static const size_t wake = MAX_SENSORS;
	static const char WAKE_MESSAGE = 'W';
	struct pollfd mPollFds[MAX_SENSORS+1];
	int mWritePipeFd;
	SensorBase* mSensors[MAX_SENSORS];
	mutable Mutex mLock;
};

/*****************************************************************************/

sensors_poll_context_t::sensors_poll_context_t()
{
	int number;
	int i;
	const struct sensor_t *slist;
	const struct SensorContext *context;
	NativeSensorManager& sm(NativeSensorManager::getInstance());

	number = sm.getSensorList(&slist);

	/* use the dynamic sensor list */
	for (i = 0; i < number; i++) {
		context = sm.getInfoByHandle(slist[i].handle);

		mPollFds[i].fd = (context == NULL) ? -1 : context->data_fd;
		mPollFds[i].events = POLLIN;
		mPollFds[i].revents = 0;
	}

	ALOGI("The avaliable sensor handle number is %d",i);
	int wakeFds[2];
	int result = pipe(wakeFds);
	ALOGE_IF(result<0, "error creating wake pipe (%s)", strerror(errno));
	fcntl(wakeFds[0], F_SETFL, O_NONBLOCK);
	fcntl(wakeFds[1], F_SETFL, O_NONBLOCK);
	mWritePipeFd = wakeFds[1];

	mPollFds[number].fd = wakeFds[0];
	mPollFds[number].events = POLLIN;
	mPollFds[number].revents = 0;
}

sensors_poll_context_t::~sensors_poll_context_t() {
	NativeSensorManager& sm(NativeSensorManager::getInstance());
	int number = sm.getSensorCount();

	close(mPollFds[number].fd);
	close(mWritePipeFd);
}

int sensors_poll_context_t::activate(int handle, int enabled) {
	int err = -1;
	NativeSensorManager& sm(NativeSensorManager::getInstance());
	Mutex::Autolock _l(mLock);

	err = sm.activate(handle, enabled);
	if (enabled && !err) {
		const char wakeMessage(WAKE_MESSAGE);
		int result = write(mWritePipeFd, &wakeMessage, 1);
		ALOGE_IF(result<0, "error sending wake message (%s)", strerror(errno));
	}

	return err;
}

int sensors_poll_context_t::setDelay(int handle, int64_t ns) {
	int err = -1;
	NativeSensorManager& sm(NativeSensorManager::getInstance());
	Mutex::Autolock _l(mLock);

	err = sm.setDelay(handle, ns);

	return err;
}

int sensors_poll_context_t::pollEvents(sensors_event_t* data, int count)
{
	int nbEvents = 0;
	int n = 0;
	NativeSensorManager& sm(NativeSensorManager::getInstance());
	const sensor_t *slist;
	int number = sm.getSensorList(&slist);

	do {
		// see if we have some leftover from the last poll()
		for (int i = 0 ; count && i < number ; i++) {
			if ((mPollFds[i].revents & POLLIN) || (sm.hasPendingEvents(slist[i].handle))) {
				Mutex::Autolock _l(mLock);
				int nb = sm.readEvents(slist[i].handle, data, count);
				if (nb < 0) {
					ALOGE("readEvents failed.(%d)", errno);
					return nb;
				}
				if (nb <= count) {
					// no more data for this sensor
					mPollFds[i].revents = 0;
				}
				count -= nb;
				nbEvents += nb;
				data += nb;
			}
		}

		if (count) {
			// we still have some room, so try to see if we can get
			// some events immediately or just wait if we don't have
			// anything to return
			do {
				n = poll(mPollFds, number + 1, nbEvents ? 0 : -1);
			} while (n < 0 && errno == EINTR);
			if (n<0) {
				ALOGE("poll() failed (%s)", strerror(errno));
				return -errno;
			}
			if (mPollFds[number].revents & POLLIN) {
				char msg;
				int result = read(mPollFds[number].fd, &msg, 1);
				ALOGE_IF(result<0, "error reading from wake pipe (%s)", strerror(errno));
				ALOGE_IF(msg != WAKE_MESSAGE, "unknown message on wake queue (0x%02x)", int(msg));
				mPollFds[number].revents = 0;
			}
		}
		// if we have events and space, go read them
	} while (n && count);

	return nbEvents;
}


int sensors_poll_context_t::batch(int handle, int sample_ns, int latency_ns)
{
	NativeSensorManager& sm(NativeSensorManager::getInstance());
	Mutex::Autolock _l(mLock);

	return sm.batch(handle, sample_ns, latency_ns);
}

int sensors_poll_context_t::flush(int handle)
{
	NativeSensorManager& sm(NativeSensorManager::getInstance());
	Mutex::Autolock _l(mLock);
	int ret = sm.flush(handle);

       /*wake up poll since meta_data need to be reported*/
	const char wakeMessage(WAKE_MESSAGE);
	int result = write(mWritePipeFd, &wakeMessage, 1);
	ALOGE_IF(result<0, "error sending wake message (%s)", strerror(errno));

	return ret;
}
/*****************************************************************************/

static int poll__close(struct hw_device_t *dev)
{
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	if (ctx) {
		delete ctx;
	}
	return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev,
		int handle, int enabled) {
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	return ctx->activate(handle, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t *dev,
		int handle, int64_t ns) {
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	return ctx->setDelay(handle, ns);
}



static int poll__poll(struct sensors_poll_device_t *dev,
		sensors_event_t* data, int count) {
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	return ctx->pollEvents(data, count);
}



#if defined(SENSORS_DEVICE_API_VERSION_1_3)
static int poll__batch(struct sensors_poll_device_1 *dev,
		int handle, int /*flags*/, int64_t sample_ns,
		int64_t latency_ns) {
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	return ctx->batch(handle, sample_ns, latency_ns);
}

static int poll__flush(struct sensors_poll_device_1 *dev,
		int handle) {
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	return ctx->flush(handle);
}
#endif

/*****************************************************************************/

/** Open a new instance of a sensor device using name */
static int open_sensors(const struct hw_module_t* module, const char*,
						struct hw_device_t** device)
{
		int status = -EINVAL;
		sensors_poll_context_t *dev = new sensors_poll_context_t();
		NativeSensorManager& sm(NativeSensorManager::getInstance());

		memset(&dev->device, 0, sizeof(sensors_poll_device_1_ext_t));

		dev->device.common.tag = HARDWARE_DEVICE_TAG;
#if defined(SENSORS_DEVICE_API_VERSION_1_3)
		ALOGI("Sensors device API version 1.3 supported\n");
		dev->device.common.version = SENSORS_DEVICE_API_VERSION_1_3;
#else
		dev->device.common.version = SENSORS_DEVICE_API_VERSION_0_1;
#endif
		dev->device.common.module   = const_cast<hw_module_t*>(module);
		dev->device.common.close	= poll__close;
		dev->device.activate		= poll__activate;
		dev->device.setDelay		= poll__setDelay;
		dev->device.poll		= poll__poll;
#if defined(SENSORS_DEVICE_API_VERSION_1_3)
		dev->device.batch		= poll__batch;
		dev->device.flush		= poll__flush;
#endif

		*device = &dev->device.common;
		status = 0;

		return status;
}


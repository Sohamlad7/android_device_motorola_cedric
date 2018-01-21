/*
 * Copyright (C) 2015 Motorola Mobility
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

#ifndef ANDROID_SENSOR_BASE_H
#define ANDROID_SENSOR_BASE_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/limits.h>
#include <string.h>
#include <vector>


/*****************************************************************************/

struct sensors_event_t;

class SensorBase {
public:
	SensorBase(
		const char* dev_name,
		const char* data_name,
		const char* mot_data_name);

	virtual ~SensorBase();

	SensorBase(const SensorBase& that) = delete;
	SensorBase& operator=(SensorBase const&) = delete;
	SensorBase(SensorBase &&) = delete;

	/**
	 * Appends up to count events to the data array. If count is not
	 * sufficiently large to hold all the pending events, subsequent calls to
	 * hasPendingEvents() should return true.
	 *
	 * @param data An array to which sensor events can be added.
	 * @param count The size of the data array. May be 0 in some cases.
	 *
	 * @return The number of events that were added to the data array. Must
	 * always be <= count.
	 */
	virtual int readEvents(sensors_event_t* data, int count) = 0;

	/**
	 * A given driver (derived class) may be registered to listen for data on
	 * multiple file descriptors. The derived class may override this method if
	 * it cares to know what file descriptor caused the poll() to exit.
	 *
	 * @param data A sensors_event_t array holding the events to be read.
	 * @param count The size of the data array.
	 * @param fd The file descriptor which caused poll() to exit. This may be
	 * -1 in some cases (for example when this function is called to finish
	 *  reading partially read events because hasPendingEvents() returned
	 *  true).
	 */
	virtual int readEvents(sensors_event_t* data, int count, int fd) {
		(void)fd;
		return readEvents(data, count);
	}
	virtual bool hasPendingEvents() const;
	virtual int getFd() const;

	/* When this function is called, increments the reference counter. */
	virtual int setEnable(int32_t handle, int enabled) = 0;
	/**
	 * Sets a sensor’s parameters, including sampling frequency and maximum
	 * report latency. This function can be called while the sensor is
	 * activated, in which case it must not cause any sensor measurements to
	 * be lost: transitioning from one sampling rate to the other cannot cause
	 * lost events, nor can transitioning from a high maximum report latency to
	 * a low maximum report latency.
	 * See the Batching sensor results page for details:
	 * http://source.android.com/devices/sensors/batching.html
	 */
	virtual int batch(int32_t handle, int32_t flags, int64_t ns, int64_t timeout) = 0;
	virtual int flush(int32_t handle) = 0;
	virtual bool hasSensor(int handle) = 0;

	/** Appends to the list the sensors handled by this object. */
	virtual void getSensorsList(std::vector<struct sensor_t> &list) = 0;

protected:
	const char* dev_name;
	// A /dev/input file that uses the input event kernel subsystem
	const char* data_name;
	// A /dev file that will be read directly (not using the input subsystem)
	const char* mot_data_name;
	int dev_fd;
	int data_fd;

	/* Used for sensors reported through the input (keyboard/mouse) kernel
	 * subsystem. */
	int openInput(const char* inputName);
	static int64_t getTimestamp();

	static int64_t timevalToNano(timeval const& t) {
		return t.tv_sec*1000000000LL + t.tv_usec*1000;
	}

	int open_device();
	int close_device();
	int write_sys_attribute(
		char const *path, char const *value, int bytes);
};

/*****************************************************************************/

#endif  // ANDROID_SENSOR_BASE_H

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

#ifndef ANDROID_SENSOR_BASE_H
#define ANDROID_SENSOR_BASE_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <linux/input.h>

#include <hardware/hardware.h>
#include <hardware/sensors.h>


/*****************************************************************************/

struct sensors_event_t;
struct SensorContext;

class SensorBase {
protected:
	const char*	dev_name;
	const char*	data_name;
	char		input_name[PATH_MAX];
	int		dev_fd;
	int		data_fd;
	int64_t report_time;
	bool mUseAbsTimeStamp;
	sensors_meta_data_event_t	meta_data;
	char input_sysfs_path[PATH_MAX];
	int input_sysfs_path_len;
	int mEnabled;
	int mHasPendingMetadata;

	int openInput(const char* inputName);
	static int64_t getTimestamp();


	static int64_t timevalToNano(timeval const& t) {
		return t.tv_sec*1000000000LL + t.tv_usec*1000;
	}

	int open_device();
	int close_device();

public:
			SensorBase(const char* dev_name, const char* data_name,
					const struct SensorContext* context = NULL);

	virtual ~SensorBase();

	virtual int readEvents(sensors_event_t* data, int count) = 0;
	virtual int injectEvents(sensors_event_t* data, int count);
	virtual bool hasPendingEvents() const;
	virtual int getFd() const;
	virtual int setDelay(int32_t handle, int64_t ns);
	virtual int enable(int32_t handle, int enabled) = 0;
	virtual int setLatency(int32_t handle, int64_t ns);
	virtual int flush(int32_t handle);
};

/*****************************************************************************/

#endif  // ANDROID_SENSOR_BASE_H

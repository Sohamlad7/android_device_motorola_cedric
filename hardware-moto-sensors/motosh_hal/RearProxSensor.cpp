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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <string.h>

#include <cutils/log.h>
#include <android-base/macros.h>

#include "RearProxSensor.h"

/*****************************************************************************/

RearProxSensor::RearProxSensor()
	: SensorBase("", "Rear proximity sensor", ""),
	mEnabled(0),
	mPendingMask(0),
	mInputReader(8),
	mFlushEnabled(0)
{

	memset(&mFlushEvents, 0, sizeof(mFlushEvents));
	mPendingEvents.version = sizeof(sensors_event_t);
	mPendingEvents.sensor = SENSORS_HANDLE_BASE + ID_RP;
	mPendingEvents.type = SENSOR_TYPE_PROXIMITY;


	mFlushEvents.version = META_DATA_VERSION;
	mFlushEvents.sensor = 0;
	mFlushEvents.type = SENSOR_TYPE_META_DATA;
	mFlushEvents.reserved0 = 0;
	mFlushEvents.timestamp = 0;
	mFlushEvents.meta_data.what = META_DATA_FLUSH_COMPLETE;
	mFlushEvents.meta_data.sensor = SENSORS_HANDLE_BASE + ID_RP;

	ALOGD("rearprox opening enable_ps_sensor");
	// read the actual value of all sensors if they're enabled already
	int fd;
	char buff[20];
	struct input_absinfo absinfo;


	fd = open("/sys/kernel/range/enable_sar", O_RDONLY);
	if (fd >= 0) {
		read(fd, buff, 19);
		buff[19] = '\0';

		if (buff[0] == '\1') {
			int fd1 = open("/sys/kernel/range/near", O_RDONLY);
			read(fd1, buff, 19);
			buff[19] = '\0';
			mPendingEvents.distance = buff[0];
		}

		close(fd);
	}
	else
		ALOGE("rearprox error opening enable_ps_sensor");
}

RearProxSensor::~RearProxSensor() {
}

int RearProxSensor::setEnable(int32_t handle, int en)
{
	char buff[2];
	if (SENSORS_HANDLE_BASE + ID_RP != handle)
		return -EINVAL;

	int newState = en ? 1 : 0;
	int err = 0;
	if (newState) {
		buff[0] = '1';
		mEnabled = 1;
	}
	else {
		buff[0] = '0';
		mEnabled = 0;
	}
	ALOGD("rearprox setEnable with %d", en);
	buff[1] = '\0';
	int	fd = open("/sys/kernel/range/enable_sar", O_WRONLY);
	if (fd >= 0) {
		write(fd, buff, 2);
		ALOGD("rearprox  opening prox_enable %s", buff);
		close(fd);
	}
	else {
		ALOGE("rearprox error opening prox_enable");
		err = -EIO;
	}

	return err;
}

int RearProxSensor::getEnable(int32_t handle)
{
	UNUSED(handle);
	return mEnabled;
}

int RearProxSensor::batch(int32_t handle, int flags,
		int64_t sampling_period_ns, int64_t max_report_latency_ns)
{
	(void)flags;
	(void)max_report_latency_ns;
	int delay;
	char buff[32];
	int num_chars;
	int fd;

	if (SENSORS_HANDLE_BASE + ID_RP != handle)
		return -EINVAL;

	if (sampling_period_ns < 0)
		return -EINVAL;

	ALOGD("rearprox batch");
	delay = sampling_period_ns / 1000000;
	num_chars = snprintf(buff, (size_t)32, "%d", delay);
	if ((num_chars < 0) || (num_chars >= 32)) {
		ALOGE("rearprox invalid delay value %d\n", delay);
		return -EINVAL;
	}

	fd = open("/sys/kernel/range/set_delay_ms", O_WRONLY);
	if (fd >= 0) {
		write(fd, buff, num_chars + 1);
		close(fd);
		ALOGD("rearprox poll interval: %s\n", buff);
	}
	else {
		ALOGE("rearprox error opening set_delay");
		return -EIO;
	}

	return 0;
}

int RearProxSensor::readEvents(sensors_event_t* data, int count)
{
	if (count < 1)
		return 0;
	int numEventReceived = 0;
	input_event const* event;

	ssize_t n = mInputReader.fill(data_fd);
	if (n < 0) {
		ALOGE("RearProxSensor: read error %d, dropped events", n);
		return 0;
	}

	while (count && mInputReader.readEvent(&event)) {
		int type = event->type;
		if (type == EV_ABS) {
			if(event->code == ABS_MISC
			&& event->value == REAR_PROX_DT_FLUSH
			&& count > 0) {
				memcpy(data, &mFlushEvents, sizeof(sensors_event_t));
				data++;
				count--;
				numEventReceived++;
			} else
				processEvent(event->code, event->value);
		}
		else if (type == EV_SYN) {
			int64_t time = timevalToNano(event->time);
			*data++ = mPendingEvents;
			count--;
			numEventReceived++;
		}
		else {
			ALOGD("RearProxSensor: unknown event (type=%d, code=%d)",
				type, event->code);
		}
		mInputReader.next();
	}

	return numEventReceived;
}

void RearProxSensor::processEvent(int code, int value)
{
	if (code == ABS_DISTANCE)
		mPendingEvents.distance = value;
}

int RearProxSensor::flush(int32_t handle)
{
	char buff[2];

	ALOGD("RearProxSensor: flush enter");
	if (SENSORS_HANDLE_BASE + ID_RP != handle)
		return -EINVAL;

	buff[0] = '2';
	buff[1] = '\0';
	int fd = open("/sys/kernel/range/enable_sar", O_WRONLY);
	if (fd >= 0) {
		write(fd, buff, 2);
		close(fd);
	}
	else {
		ALOGE("RearProxSensor: error opening enable_sar");
		return -EIO;
	}

	mFlushEnabled = 1;
	return 0;
}

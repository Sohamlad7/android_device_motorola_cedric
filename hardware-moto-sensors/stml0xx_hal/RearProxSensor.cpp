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
static const char *Proximity_sensor_data_name[2] = { "Rear proximity sensor1","Rear proximity sensor2"};

RearProxSensor::RearProxSensor(int n)
#ifdef _ENABLE_REARPROX_2
	: SensorBase("", Proximity_sensor_data_name[n], ""),
#else
	: SensorBase("", "Rear proximity sensor", ""),
#endif
	mEnabled(0),
	mPendingMask(0),
	mInputReader(8),
	mFlushEnabled(0)
{

	memset(&mFlushEvents, 0, sizeof(mFlushEvents));
	mPendingEvents.version = sizeof(sensors_event_t);
	if (n == 0)
		mPendingEvents.sensor = ID_RP;
#ifdef _ENABLE_REARPROX_2
	if (n == 1)
		mPendingEvents.sensor = ID_RP_2;
#endif
	mPendingEvents.type = SENSOR_TYPE_PROXIMITY;

#ifdef _ENABLE_REARPROX_2
	numrp = n;
#endif

	mFlushEvents.version = META_DATA_VERSION;
	mFlushEvents.sensor = 0;
	mFlushEvents.type = SENSOR_TYPE_META_DATA;
	mFlushEvents.reserved0 = 0;
	mFlushEvents.timestamp = 0;
	mFlushEvents.meta_data.what = META_DATA_FLUSH_COMPLETE;
	if (n == 0)
		mFlushEvents.meta_data.sensor = ID_RP;
#ifdef _ENABLE_REARPROX_2
	if (n == 1)
		mFlushEvents.meta_data.sensor = ID_RP_2;
#endif
	mDelays = 2000000000; // 2000 ms by default
	ALOGE("rearprox opening enable_ps_sensor %d\n",n);

	// read the actual value of all sensors if they're enabled already
	int fd;
	char buff[20];
	if (n == 0)
#ifdef _ENABLE_REARPROX_2
		fd = open("/sys/class/i2c-dev/i2c-2/device/2-0049/enable_sar", O_RDONLY);
#else
		fd = open("/sys/kernel/range/enable_sar", O_RDONLY);
#endif
	if (n == 1)
		fd = open("/sys/class/i2c-dev/i2c-6/device/6-0049/enable_sar", O_RDONLY);
	if (fd >= 0) {
		read(fd, buff, 19);
		buff[19] = '\0';

		if (buff[0] == '\1') {
			int fd1;
			if (n == 0)
#ifdef _ENABLE_REARPROX_2
				fd1 = open("/sys/class/i2c-dev/i2c-2/device/2-0049/near", O_RDONLY);
#else
				fd1 = open("/sys/kernel/range/near", O_RDONLY);
#endif
			if (n == 1)
				fd1 = open("/sys/class/i2c-dev/i2c-6/device/6-0049/near", O_RDONLY);
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
#ifdef _ENABLE_REARPROX_2
	if (ID_RP != handle && ID_RP_2 != handle)
#else
	if (ID_RP != handle)
#endif
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
	ALOGE("rearprox enable");
	buff[1] = '\0';

	int fd = 0;
#ifdef _ENABLE_REARPROX_2
	if (numrp == 0)
		fd = open("/sys/class/i2c-dev/i2c-2/device/2-0049/enable_sar", O_WRONLY);
	if (numrp == 1)
		fd = open("/sys/class/i2c-dev/i2c-6/device/6-0049/enable_sar", O_WRONLY);
#else
	fd = open("/sys/kernel/range/enable_sar", O_WRONLY);
#endif

	if (fd >= 0) {
		write(fd, buff, 2);
		ALOGE("rearprox  opening prox_enable %s", buff);
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

int RearProxSensor::setDelay(int32_t handle, int64_t ns)
{
	UNUSED(handle);
	int delay;
	char buff[32];
	int num_chars;
	int fd = 0;

#ifdef _ENABLE_REARPROX_2
	if (ID_RP != handle && ID_RP_2 != handle)
#else
	if (ID_RP != handle)
#endif
		return -EINVAL;


	if (ns < 0)
		return -EINVAL;

	mDelays = ns;
	ALOGE("rearprox setDelay");
	delay = ns / 1000000;
	num_chars = snprintf(buff, (size_t)32, "%d", delay);
	if ((num_chars < 0) || (num_chars >= 32)) {
		ALOGE("rearprox invalid delay value %d\n", delay);
		return -EINVAL;
	}

#ifdef _ENABLE_REARPROX_2
	if (numrp == 0)
		fd = open("/sys/class/i2c-dev/i2c-2/device/2-0049/set_delay_ms", O_WRONLY);
	if (numrp == 1)
		fd = open("/sys/class/i2c-dev/i2c-6/device/6-0049/set_delay_ms", O_WRONLY);
#else
	fd = open("/sys/kernel/range/set_delay_ms", O_WRONLY);
#endif

	if (fd >= 0) {
		write(fd, buff, num_chars + 1);
		close(fd);
		ALOGE("rearprox poll interval: %s\n", buff);
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
	ALOGE("rearprox readEvents");
	int numEventReceived = 0;
	input_event const* event;

	ssize_t n = mInputReader.fill(data_fd);
	if (n < 0) {
		ALOGE("RearProxSensor: read error %d, dropped events", n);
		return 0;
	}

	while (count && mInputReader.readEvent(&event)) {
		int type = event->type;
		if (type == EV_ABS && event->value != -1) {
			processEvent(event->code, event->value);
		}
		else if (type == EV_SYN) {
			int64_t time = timevalToNano(event->time);
			*data++ = mPendingEvents;
			count--;
			numEventReceived++;
		}
		else {
			ALOGE("RearProxSensor: unknown event (type=%d, code=%d, value=%d)",
				type, event->code, event->value);
		}
		mInputReader.next();
	}

	return numEventReceived;
}

void RearProxSensor::processEvent(int code, int value)
{
	if (code == ABS_DISTANCE){
		mPendingEvents.distance = value;
		ALOGE("RearProxSensor: processEvent distance:%f", mPendingEvents.distance);
	}
}

int RearProxSensor::flush(int32_t handle)
{
	UNUSED(handle);

	mFlushEnabled = 1;
	return 0;
}

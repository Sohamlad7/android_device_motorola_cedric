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

#include "CapSense.h"

CapSense CapSense::self;
/*****************************************************************************/

CapSense::CapSense()
	: SensorBase("", "SX9310 Cap Touch", ""),
	mEnabled(0),
	mPendingMask(0),
	mInputReader(8),
	mFlushEnabled(0)
{

	memset(&mFlushEvents, 0, sizeof(mFlushEvents));
	mPendingEvents.version = sizeof(sensors_event_t);
	mPendingEvents.sensor = ID_CS;
	mPendingEvents.type = SENSOR_TYPE_MOTO_CAPSENSE;


	mFlushEvents.version = META_DATA_VERSION;
	mFlushEvents.sensor = 0;
	mFlushEvents.type = SENSOR_TYPE_META_DATA;
	mFlushEvents.reserved0 = 0;
	mFlushEvents.timestamp = 0;
	mFlushEvents.meta_data.what = META_DATA_FLUSH_COMPLETE;
	mFlushEvents.meta_data.sensor = ID_CS;

	mDelays = 2000000000; // 2000 ms by default
}

CapSense::~CapSense() {
}

CapSense* CapSense::getInstance()
{
	return &self;
}

int CapSense::setEnable(int32_t handle, int en)
{
	char buff[2];
	if (ID_CS != handle)
		return -EINVAL;

	int newState = en ? 1 : 0;
	int err = 0;
	if (newState) {
		buff[0] = '1';
		mEnabled = 1;
	} else {
		buff[0] = '0';
		mEnabled = 0;
	}
	ALOGD("CapSense::setEnable enable:%d", mEnabled);

	buff[1] = '\0';
	int fd = open("/sys/class/capsense/enable", O_WRONLY);
	if (fd >= 0) {
		write(fd, buff, 2);
		close(fd);
	} else {
		ALOGE("CapSense opening failed");
		err = -EIO;
	}

	return err;
}

int CapSense::getEnable(int32_t handle)
{
    UNUSED(handle);
	return mEnabled;
}

int CapSense::setDelay(int32_t handle, int64_t ns)
{
	UNUSED(handle);

	if (ID_CS != handle)
		return -EINVAL;

	if (ns < 0)
		return -EINVAL;

	mDelays = ns/1000000;

	ALOGD("CapSense::setDelay delay:%lld", mDelays);

	return 0;
}

int CapSense::readEvents(sensors_event_t* data, int count)
{
	if (count < 1)
		return 0;

	int numEventReceived = 0;
	input_event const* event;

	ssize_t n = mInputReader.fill(data_fd);
	if (n < 0) {
		ALOGE("CapSense: read error %d, dropped events", n);
		return 0;
	}

	while (count && mInputReader.readEvent(&event)) {
		int type = event->type;
		if (type == EV_ABS) {
			processEvent(event->code, event->value);
		}
		else if (type == EV_SYN) {
			int64_t time = timevalToNano(event->time);
			*data++ = mPendingEvents;
			count--;
			numEventReceived++;
		}
		else {
			ALOGE("CapSense: unknown event (type=%d, code=%d)",
				type, event->code);
		}
		mInputReader.next();
	}

	return numEventReceived;
}

void CapSense::processEvent(int code, int value)
{
	if ((code == ABS_DISTANCE) && (-1 != value)) {
		ALOGD("CapSense: processEvent event value=%d", value);
		mPendingEvents.distance = value;
	}
}

int CapSense::flush(int32_t handle)
{
	UNUSED(handle);

	mFlushEnabled = 1;
	return 0;
}

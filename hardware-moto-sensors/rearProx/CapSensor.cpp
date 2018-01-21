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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include "CapSensor.h"
#include "sensors.h"

#define EVENT_TYPE_CAPSENSOR		ABS_DISTANCE

#define SENSOR_TYPE_MOTO_CAPSENSE SENSOR_TYPE_DEVICE_PRIVATE_BASE + 16

#define ARRAY	3

/*****************************************************************************/

CapSensor::CapSensor(struct SensorContext *context)
: SensorBase(NULL, NULL, context),
    mInputReader(4),
    mHasPendingEvent(false)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = context->sensor->handle;
    mPendingEvent.type = SENSOR_TYPE_MOTO_CAPSENSE;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

    data_fd = context->data_fd;
    strlcpy(input_sysfs_path, context->enable_path, sizeof(input_sysfs_path));
    input_sysfs_path_len = strlen(input_sysfs_path);
}

CapSensor::~CapSensor() {
    if (mEnabled) {
        enable(0, 0);
    }
}

int CapSensor::enable(int32_t, int en) {
    int flags = en ? 1 : 0;
    char propBuf[PROPERTY_VALUE_MAX];
    property_get("sensors.capsense.loopback", propBuf, "0");
    if (strcmp(propBuf, "1") == 0) {
        mEnabled = flags;
        ALOGE("sensors.capsense.loopback is set");
        return 0;
    }

    if (flags != mEnabled) {
        int fd;

        strlcpy(&input_sysfs_path[input_sysfs_path_len], SYSFS_ENABLE,
                sizeof(input_sysfs_path) - input_sysfs_path_len);
        fd = open(input_sysfs_path, O_RDWR);
        if (fd >= 0) {
            char buf[2];
            buf[1] = 0;
            if (flags) {
                buf[0] = '1';
            } else {
                buf[0] = '0';
            }
            write(fd, buf, sizeof(buf));
            close(fd);
            mEnabled = flags;
            return 0;
        } else {
            ALOGE("open %s failed.(%s)\n", input_sysfs_path, strerror(errno));
            return -1;
        }
    } else if (flags) {
        mHasPendingEvent = true;
    }
    return 0;
}

bool CapSensor::hasPendingEvents() const {
    return mHasPendingEvent || mHasPendingMetadata;
}

int CapSensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
    return -EINVAL;

    if (mHasPendingEvent) {
        mHasPendingEvent = false;
        mPendingEvent.timestamp = getTimestamp();
        *data = mPendingEvent;
        return mEnabled ? 1 : 0;
    }

    if (mHasPendingMetadata) {
        mHasPendingMetadata--;
        meta_data.timestamp = getTimestamp();
        *data = meta_data;
        return mEnabled ? 1 : 0;
    }

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
    return n;

    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {
            if (event->code == EVENT_TYPE_CAPSENSOR) {
                if (event->value != -1) {
                    // FIXME: not sure why we're getting -1 sometimes
                    mPendingEvent.distance = indexToValue(event->value);
                    ALOGD("CapSensor:  mPendingEvent.distance=%f\n",mPendingEvent.distance);
                }
            }
        } else if (type == EV_SYN) {
            switch ( event->code ) {
                case SYN_REPORT:
                    if(mUseAbsTimeStamp != true) {
                        mPendingEvent.timestamp = timevalToNano(event->time);
                    }
                    if (mEnabled) {
                        *data++ = mPendingEvent;
                        count--;
                        numEventReceived++;
                    }
                break;
            }
        } else {
            ALOGE("CapSensor: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }

    return numEventReceived;
}

int CapSensor::setDelay(int32_t, int64_t ns)
{
    int fd;
    char propBuf[PROPERTY_VALUE_MAX];
    char buf[80];
    int len;

    property_get("sensors.capsense.loopback", propBuf, "0");
    if (strcmp(propBuf, "1") == 0) {
        ALOGE("sensors.capsense.loopback is set");
        return 0;
    }
    int delay_ms = ns / 1000000;
    strlcpy(&input_sysfs_path[input_sysfs_path_len],
    SYSFS_POLL_DELAY, SYSFS_MAXLEN);
    fd = open(input_sysfs_path, O_RDWR);
    if (fd < 0) {
        ALOGE("open %s failed.(%s)\n", input_sysfs_path, strerror(errno));
        return -1;
    }
    snprintf(buf, sizeof(buf), "%d", delay_ms);
    len = write(fd, buf, ssize_t(strlen(buf)+1));
    if (len < ssize_t(strlen(buf) + 1)) {
        ALOGE("write %s failed\n", buf);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

float CapSensor::indexToValue(size_t index) const
{
    return index;
}


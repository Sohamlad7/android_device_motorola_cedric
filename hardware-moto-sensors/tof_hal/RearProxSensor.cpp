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
#include <string>
#include <stdlib.h>
#include <linux/input.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <android-base/macros.h>

#include "RearProxSensor.h"

#define PATH_ENABLE_SAR       "enable_sar"
#define PATH_SET_DELAY_MS     "set_delay_ms"
#define PATH_DO_FLUSH         "do_flush"
#define PATH_AUTONOMOUS_CFG   "autonomous_config"
#define PATH_BASE             "/sys/devices/virtual/laser"
#define PATH_ATTR_NAME        "name"

#define MODE_SAR_RESUME       '2'
#define MODE_SAR_ON           '1'
#define MODE_SAR_OFF          '0'

#define DETECT_MODE_CROSS_LOW 0
#define DETECT_MODE_CROSS_HIGH 1
#define DETECT_WEIGHT_CEILING 2
#define DETECT_DEFAULT_INTERVAL_MS 500
#define DETECT_DEFAULT_LOW_THRESHOLD_MM 70
#define DETECT_DEFAULT_HIGH_THRESHOLD_MM 75
#define DETECT_DISTANCE_CLOSE 1
#define DETECT_DISTANCE_FAR 10

#define PROPERTY_TOF_POLL_INTERVAL_MS "ro.sensors.tof.interval_ms"
#define VL53L1_INPUT_DEVICE_NAME "STM VL53L1 proximity sensor"

/*****************************************************************************/
using namespace std;

RearProxSensor::RearProxSensor()
    : SensorBase("", VL53L1_INPUT_DEVICE_NAME, ""),
    mEnabled(0),
    mFlushEnabled(0),
    mDetectInterval(DETECT_DEFAULT_INTERVAL_MS),
    mDetectDistance(DETECT_DISTANCE_CLOSE),
    mDetectWeight(0),
    mDetectMode(DETECT_MODE_CROSS_HIGH),
    mCurrentDistance(0),
    mTimestampUpdated(false),
    mSendData(false),
    mInputReader(8)
{
    mPendingEvents.version = sizeof(sensors_event_t);
    mPendingEvents.sensor = ID_RP;
    mPendingEvents.type = SENSOR_TYPE_PROXIMITY;
}

RearProxSensor::~RearProxSensor()
{
}

int RearProxSensor::findPathByAttr()
{
    char aval[64] = {0};
    string attr_name("name");
    int rc;
    int found = 0;
    int fd;
    DIR *dir;
    struct dirent *item;

    dir = opendir(PATH_BASE);
    if (!dir) {
        ALOGE("Unable to open '%s'", PATH_BASE);
        return 0;
    }

    while (NULL != (item = readdir(dir))) {
        mFilePath.clear();
        if (item->d_type != DT_DIR && item->d_type != DT_LNK)
            continue;

        if (item->d_name[0] == '.')
            continue;

        mFilePath.append(PATH_BASE);
        mFilePath.append("/");
        mFilePath.append(item->d_name);
        mFilePath.append("/");
        mFilePath.append(attr_name);

        ALOGD("checking %s...", mFilePath.c_str());
        fd = open(mFilePath.c_str(), O_RDONLY);
        if (fd < 0) {
            ALOGE("Unable to open '%s'", mFilePath.c_str());
            continue;
        }
        memset(aval, 0, sizeof(aval));
        rc = read(fd, aval, sizeof(aval) - 1);
        close(fd);
        if (rc < 0) {
            ALOGE("Unable to read '%s'", mFilePath.c_str());
            continue;
        }
        found = (strstr(aval, VL53L1_INPUT_DEVICE_NAME) != NULL);
        if (found) {
            int len = mFilePath.length();
            mFilePath.erase(len - attr_name.length() - 1, attr_name.length() + 1);
            ALOGD("found %s in path %s", VL53L1_INPUT_DEVICE_NAME, mFilePath.c_str());
            break;
        }
    }
    closedir(dir);
    return found;
}

int RearProxSensor::setSARConfig(uint32_t mode)
{
    int fd = 0;
    int err = 0;
    char buff[64] = {0};

    if(mode != DETECT_MODE_CROSS_HIGH && mode != DETECT_MODE_CROSS_LOW)
        return -EINVAL;

    ALOGD("Enter setSARConfig with mode %s", mode?"cross_high":"cross_low");

    int intrNotarget = mode?1:0;

    sprintf(buff, "%d 1 %d %d %d %d 0 0.000000 0.000000\n",
            mDetectInterval,
            intrNotarget,
            mode,
            DETECT_DEFAULT_HIGH_THRESHOLD_MM,
            DETECT_DEFAULT_LOW_THRESHOLD_MM);

    string path;
    path.append(mFilePath);
    path.append("/");
    path.append(PATH_AUTONOMOUS_CFG);

    fd = open(path.c_str(), O_WRONLY);
    if (fd >= 0) {
        write(fd, buff, strlen(buff));
        close(fd);
        mDetectMode = mode;
        ALOGD("New autonomous mode %d setting %s", mode, buff);
    } else {
        ALOGE("Fail to open %s with error %s", path.c_str(), strerror(errno));
        err = -EIO;
    }

    return err;
}

int RearProxSensor::enableSAR(char enable)
{
    char buff[2] = {0};
    int err = 0;
    int fd = 0;
    string path;

    path.append(mFilePath);
    path.append("/");
    path.append(PATH_ENABLE_SAR);
    buff[0] = enable;
    fd = open(path.c_str(), O_WRONLY);
    if (fd >= 0) {
        write(fd, buff, 2);
        ALOGD("Enable SAR mode with %c", enable);
        close(fd);
    } else {
        ALOGE("Fail to open %s with error %s", path.c_str(), strerror(errno));
        err = -EIO;
    }

    return err;
}

int RearProxSensor::setEnable(int32_t handle, int en)
{
    char newState = en ? MODE_SAR_ON : MODE_SAR_OFF;
    int err = 0;

    ALOGD("setEnable with %d", en);

    if(mFilePath.length() == 0)
        findPathByAttr();

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    if (property_get(PROPERTY_TOF_POLL_INTERVAL_MS, value, NULL) > 0)
        mDetectInterval = atoi(value);

    switch(handle) {
        case (ID_RP):
            mEnabled = en;
            if (mEnabled) {
                // start from far distance and detect low threshod
                mDetectDistance = DETECT_DISTANCE_CLOSE;
                setSARConfig(DETECT_MODE_CROSS_HIGH);
            }
            enableSAR(newState);
            break;
        default:
            err = -EINVAL;
            break;
    }

    return err;
}

int RearProxSensor::batch(int32_t handle, int flags,
        int64_t sampling_period_ns, int64_t max_report_latency_ns)
{
    (void)flags;
    (void)max_report_latency_ns;
    int delay;
    const int buffSz = 32;
    char buff[buffSz];
    int num_chars;
    int fd = 0;

    if (sampling_period_ns < 0)
        return -EINVAL;

    ALOGD("batch enter");
    delay = sampling_period_ns / 1000000;
    num_chars = snprintf(buff, (size_t)buffSz, "%d", delay);
    if ((num_chars < 0) || (num_chars >= buffSz)) {
        ALOGE("Invalid delay value %d\n", delay);
        return -EINVAL;
    }

    string path;
    path.append(mFilePath);
    path.append("/");
    path.append(PATH_SET_DELAY_MS);

    switch(handle) {
        case (ID_RP):
            fd = open(path.c_str(), O_WRONLY);
            if (fd >= 0) {
                write(fd, buff, num_chars + 1);
                close(fd);
                ALOGD("Set poll interval: %s\n", buff);
            }
            else {
                ALOGE("Error opening %s", path.c_str());
                return -EIO;
            }
            break;
        default:
            return -EINVAL;
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
        ALOGE("Read error %zd, dropped events", n);
        return 0;
    }

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {
            if(mEnabled == 1)
                processEvent(event->code, event->value);
        }
        else if (type == EV_SYN) {
            if(mFlushEnabled == 1) {
                data->version = META_DATA_VERSION;
                data->sensor = 0;
                data->type = SENSOR_TYPE_META_DATA;
                data->reserved0 = 0;
                data->timestamp = 0;
                data->meta_data.what = META_DATA_FLUSH_COMPLETE;
                data->meta_data.sensor = ID_RP;
                data++;
                mFlushEnabled = 0;
                ALOGD("flush data is ready");
                count--;
                numEventReceived++;
            } else {
                if (mTimestampUpdated) {
                    handleSARprocess(mCurrentDistance);
                    mTimestampUpdated = false;
                }

                if (mSendData) {
                    *data++ = mPendingEvents;
                    count--;
                    numEventReceived++;
                    mSendData = false;
                }
            }
        }
        mInputReader.next();
    }

    return numEventReceived;
}

void RearProxSensor::processEvent(int code, int value)
{
    switch(code) {
        case ABS_HAT1X:
            if (!mTimestampUpdated) {
                // new distance data only
                handleSARprocess(value);
            }
            mCurrentDistance = value;
            break;
        case ABS_HAT0Y:
            // new timestamp
            mTimestampUpdated = true;
            break;
        case ABS_GAS:
            ALOGD("flush returned data");
            mFlushEnabled = 1;
            break;
        default:
            break;
    }
}

void RearProxSensor::handleSARprocess(int distance)
{
    if(distance <= DETECT_DEFAULT_LOW_THRESHOLD_MM) {
        if(mDetectDistance == DETECT_DISTANCE_FAR) {
            // get different data, update distance and wight
            mDetectDistance = DETECT_DISTANCE_CLOSE;
            mDetectWeight = 0;
        }
        // get more close data, increase weight
        mDetectWeight++;
    } else if(distance >= DETECT_DEFAULT_HIGH_THRESHOLD_MM) {
        if(mDetectDistance == DETECT_DISTANCE_CLOSE) {
            // get different data, update distance and wight
            mDetectDistance = DETECT_DISTANCE_FAR;
            mDetectWeight = 0;
        }
        // get more far data, increase weight
        mDetectWeight++;
    }

    if(mDetectWeight >= DETECT_WEIGHT_CEILING) {
        uint32_t mode = (mDetectDistance == DETECT_DISTANCE_CLOSE)?
            DETECT_MODE_CROSS_HIGH:DETECT_MODE_CROSS_LOW;
        mPendingEvents.distance = mDetectDistance;
        if(mDetectMode != mode) {
            enableSAR(MODE_SAR_OFF);
            setSARConfig(mode);
            enableSAR(MODE_SAR_RESUME);
            mSendData = true;
        }
    }
}

int RearProxSensor::flush(int32_t handle)
{
    int fd = -1;
    int ret = 0;
    ALOGD("flush enter");

    string path;
    path.append(mFilePath);
    path.append("/");
    path.append(PATH_DO_FLUSH);

    switch(handle) {
        case ID_RP:
            fd = open(path.c_str(), O_RDWR);
            if (fd >= 0) {
                char buf[4];
                sprintf(buf,"%d", 1);
                write(fd, buf, strlen(buf)+1);
                close(fd);
            } else {
                ALOGE("Failed to open %s for flush", path.c_str());
                ret = -EIO;
            }
            break;
        default:
            ret = -EINVAL;
            break;
    }
    return ret;
}

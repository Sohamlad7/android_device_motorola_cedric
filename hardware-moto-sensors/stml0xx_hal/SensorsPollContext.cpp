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
 * Copyright (C) 2015-2016 Motorola Mobility LLC
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <new>
#include <string.h>

#include <linux/input.h>

#include <cutils/atomic.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include <hardware/sensors.h>
#include "mot_sensorhub_stml0xx.h"

#include "SensorList.h"
#include "Sensors.h"
#include "SensorsPollContext.h"
#if defined(_ENABLE_MAGNETOMETER) || defined(_ENABLE_REARPROX) ||defined (_ENABLE_REARPROX_2)
#include "SensorBase.h"
#endif
#if  defined ( _ENABLE_REARPROX) || defined (_ENABLE_REARPROX_2)
#include "RearProxSensor.h"
#endif
#ifdef _ENABLE_CAPSENSE
#include "CapSense.h"
#endif
#include "HubSensors.h"

/*****************************************************************************/

SensorsPollContext SensorsPollContext::self;

SensorsPollContext::SensorsPollContext()
{
    char prop[PROPERTY_VALUE_MAX];
    char *cap_prop = {"ro.hw.capsense"};
    char *ecomp_prop = {"ro.hw.ecompass"};
    mSensors[sensor_hub] = HubSensors::getInstance();
    if (mSensors[sensor_hub]) {
        mPollFds[sensor_hub].fd = mSensors[sensor_hub]->getFd();
        mPollFds[sensor_hub].events = POLLIN;
        mPollFds[sensor_hub].revents = 0;
    } else {
        ALOGE("out of memory: new failed for HubSensors");
    }

#ifdef _ENABLE_REARPROX
    mSensors[rearprox] =new RearProxSensor(0);
    ALOGE("rearprox sensor_1 created");
    if (mSensors[rearprox]) {
        mPollFds[rearprox].fd = mSensors[rearprox]->getFd();
        mPollFds[rearprox].events = POLLIN;
        mPollFds[rearprox].revents = 0;
    } else {
        ALOGE("out of memory: new failed for rearprox sensor_1");
    }
#endif
#ifdef _ENABLE_REARPROX_2
    mSensors[rearprox_2] =new RearProxSensor(1);
    ALOGE("rearprox sensor_2 created");
    if (mSensors[rearprox_2]) {
        mPollFds[rearprox_2].fd = mSensors[rearprox_2]->getFd();
        mPollFds[rearprox_2].events = POLLIN;
        mPollFds[rearprox_2].revents = 0;
    } else {
        ALOGE("out of memory: new failed for rearprox sensor_2");
    }
#endif

#ifdef _ENABLE_MAGNETOMETER
    //if eCompass is enabled in all sku, this prop is not enforced
    if (property_get(ecomp_prop, prop,NULL) > 0 && strcmp(prop, "false") == 0) {
        ALOGD("%s = %s, don't add eCompass", ecomp_prop, prop);
    } else {
        ALOGD("add eCompass sensor");
        sSensorList.push_back(threeAxCalMagSensorType);
        sSensorList.push_back(threeAxunCalMagSensorType);
        sSensorList.push_back(orientationSensorType);
        sSensorList.push_back(geoRotationSensorType);
        sSensorList.push_back(rotationSensorType);
    }
#endif

#ifdef _ENABLE_CAPSENSE
    //make dynamic sensor
    //if capsense is enabled in all sku, this prop is not enforced
    if (property_get(cap_prop, prop,NULL) > 0 && strcmp(prop, "false") == 0) {
        ALOGD("%s = %s, don't add cap sensor",cap_prop, prop);
    } else {
        ALOGD("add cap sensor");
        sSensorList.push_back(capSensorType);
        mSensors[capsense] = CapSense::getInstance();
        if (mSensors[capsense]) {
            mPollFds[capsense].fd = mSensors[capsense]->getFd();
            mPollFds[capsense].events = POLLIN;
            mPollFds[capsense].revents = 0;
        } else {
            ALOGE("out of memory: new failed for capsense sensor");
        }
    }
#endif

    // Add all supported sensors to the mIdToSensor map
    for( unsigned int i = 0; i < sSensorList.size(); ++i ) {
        mIdToSensor.insert(std::make_pair(sSensorList[i].handle, &(sSensorList.at(i))));
    }
}

SensorsPollContext::~SensorsPollContext()
{
}

SensorsPollContext *SensorsPollContext::getInstance()
{
    return &self;
}

int SensorsPollContext::handleToDriver(int handle)
{
    switch (handle) {
        case ID_A:
#ifdef _ENABLE_GYROSCOPE
        case ID_G:
        case ID_UNCALIB_GYRO:
        case ID_GAME_RV:
        case ID_LA:
        case ID_GRAVITY:
#endif
        case ID_L:
        case ID_DR:
        case ID_P:
        case ID_FU:
        case ID_FD:
        case ID_S:
        case ID_CA:
#ifdef _ENABLE_ACCEL_SECONDARY
        case ID_A2:
#endif
#ifdef _ENABLE_CHOPCHOP
        case ID_CC:
#endif
#ifdef _ENABLE_LIFT
        case ID_LF:
#endif
#ifdef _ENABLE_PEDO
        case ID_STEP_COUNTER:
        case ID_STEP_DETECTOR:
#endif
        case ID_GLANCE_GESTURE:
        case ID_MOTO_GLANCE_GESTURE:
	case ID_MOTION_DETECT:
	case ID_STATIONARY_DETECT:
            return sensor_hub;
#ifdef _ENABLE_MAGNETOMETER
        case ID_M:
        case ID_UM:
        case ID_OR:
        case ID_GEOMAG_RV:
        case ID_RV:
#endif
            return sensor_hub;
#ifdef _ENABLE_REARPROX
        case ID_RP:
            return rearprox;
#endif
#ifdef _ENABLE_REARPROX_2
        case ID_RP_2:
            return rearprox_2;
#endif
#ifdef _ENABLE_CAPSENSE
        case ID_CS:
            return capsense;
#endif
    }
    return -EINVAL;
}

int SensorsPollContext::activate(int handle, int enabled)
{
    int drv = handleToDriver(handle);
    int err = 0;

    if (drv < 0)
        return -EINVAL;

    if( !mIdToSensor.count(handle) ) {
        ALOGE("Sensorhub hal activate: %d - %d (bad handle)", handle, enabled);
        return -EINVAL;
    }

    err = mSensors[drv]->setEnable(handle, enabled);

    return err;
}

int SensorsPollContext::setDelay(int handle, int64_t ns)
{
    int drv = handleToDriver(handle);
    int err = 0;

    if (drv < 0)
        return -EINVAL;

    if( !mIdToSensor.count(handle) ) {
        ALOGE("Sensorhub hal setDelay: %d - %" PRId64 " (bad handle)", handle, ns);
        return -EINVAL;
    }

    err = mSensors[drv]->setDelay(handle, ns);

    return err;
}

int SensorsPollContext::pollEvents(sensors_event_t* data, int count)
{
    int nbEvents = 0;
    int n = 0;
    int ret;
    int err;

    if (!data) {
        ALOGE("poll failed, data buffer is null");
        return -EINVAL;
    }

    if (count < 1) {
        ALOGE("poll failed, invalid event count %d", count);
        return -EINVAL;
    }

    while (true) {
        ret = poll(mPollFds, numSensorDrivers, nbEvents ? 0 : -1);
        err = errno;
        // Success
        if (ret >= 0)
            break;
        // EINTR is OK
        if (err == EINTR) {
            ALOGE("poll() restart (%s)", strerror(err));
            continue;
        } else {
            ALOGE("poll() failed (%s)", strerror(err));
            return 0;
        }
    }


    for (int i=0 ; count && i<numSensorDrivers ; i++) {
        //add a null detect here
        //this make daynamic sSensorList compatible with immutable array mSensors and mPollFds
        //if a sensor isn't added in mSensors, then shouldn't pollfd
        if (!mSensors[i])
            continue;
        SensorBase* const sensor(mSensors[i]);
        if ((mPollFds[i].revents & POLLIN) || (sensor->hasPendingEvents())) {
            int nb = sensor->readEvents(data, count);
            if (nb < 0) {
                ALOGE("readEvents failed %d possibly dropped events", nb);
                return nbEvents;
            }
            count -= nb;
            nbEvents += nb;
            data += nb;
            mPollFds[i].revents = 0;
        }
    }

    return nbEvents;
}

int SensorsPollContext::batch(int handle, int flags, int64_t ns, int64_t timeout)
{
    (void)flags;
    (void)timeout;
    return setDelay(handle, ns);
}

int SensorsPollContext::flush(int handle)
{
    if (!mIdToSensor.count(handle)) {
        ALOGE("Sensorhub hal flush: %d (bad handle)", handle);
        return -EINVAL;
    }

    // Have to return -EINVAL for one-shot sensors per Android spec
    if ((mIdToSensor[handle]->flags & REPORTING_MODE_MASK) == SENSOR_FLAG_ONE_SHOT_MODE) {
        return -EINVAL;
    }

    // flush for rearprox is handled by sensorhub too here,
    // this is a workaround for rearprox and flush should be
    // implemented by rearprox driver ideally
    return mSensors[sensor_hub]->flush(handle);
}

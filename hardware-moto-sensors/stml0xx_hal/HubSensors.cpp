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
 * Copyright (C) 2011-2016 Motorola Mobility LLC
 */

#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

#include <linux/stml0xx.h>

#include <cutils/log.h>

#include <hardware/mot_sensorhub_stml0xx.h>

#include "HubSensors.h"

/*****************************************************************************/

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

HubSensors HubSensors::self;

HubSensors::HubSensors()
: SensorBase(SENSORHUB_DEVICE_NAME, NULL, SENSORHUB_AS_DATA_NAME),
    mEnabled(0),
    mWakeEnabled(0),
    mPendingMask(0),
    mEnabledHandles(0),
    mPendingBug2go(0)
{
    // read the actual value of all sensors if they're enabled already
    struct input_absinfo absinfo;
    short flags16 = 0;
    uint32_t flags24 = 0;
    int i, err = 0;
    int size;
    FILE *fp;

    static_assert(MAX_SENSOR_ID < (sizeof(mEnabledHandles) * CHAR_BIT),
        "enabled handlers bit mask can NOT hold all the handles");

    memset(mErrorCnt, 0, sizeof(mErrorCnt));
#ifdef _ENABLE_GYROSCOPE
    memset(mGyroCal, 0, sizeof(mGyroCal));
#endif
    memset(mAccelCal, 0, sizeof(mAccelCal));

    open_device();

    // Initialize fusion sensor table
    for (i = 0; i < NUM_FUSION_DEVICES; i++) {
        mFusionSensors[i].enabled = false;
        mFusionSensors[i].usesAccel = false;
        mFusionSensors[i].usesGyro = false;
        mFusionSensors[i].usesMag = false;
        mFusionSensors[i].delay = USHRT_MAX;
    }
    mFusionSensors[ACCEL].usesAccel = true;
#ifdef _ENABLE_GYROSCOPE
    mFusionSensors[GYRO].usesGyro = true;
    mFusionSensors[UNCALIB_GYRO].usesGyro = true;
    mFusionSensors[GAME_RV].usesAccel = true;
    mFusionSensors[GAME_RV].usesGyro = true;
    mFusionSensors[LINEAR_ACCEL].usesAccel = true;
    mFusionSensors[LINEAR_ACCEL].usesGyro = true;
    mFusionSensors[GRAVITY].usesAccel = true;
    mFusionSensors[GRAVITY].usesGyro = true;
#endif
#ifdef _ENABLE_MAGNETOMETER
    mFusionSensors[MAG].usesAccel = true;
    mFusionSensors[MAG].usesMag = true;
    mFusionSensors[UNCALIB_MAG].usesAccel = true;
    mFusionSensors[UNCALIB_MAG].usesMag = true;
    mFusionSensors[ORIENTATION].usesAccel = true;
    mFusionSensors[ORIENTATION].usesMag = true;
    mFusionSensors[GEOMAG_RV].usesAccel = true;
    mFusionSensors[GEOMAG_RV].usesMag = true;
    mFusionSensors[ROTATION_VECT].usesAccel = true;
    mFusionSensors[ROTATION_VECT].usesMag = true;
    mFusionSensors[ROTATION_VECT].usesGyro = true;
#endif

#ifdef _ENABLE_GYROSCOPE
    if ((fp = fopen(GYRO_CAL_FILE, "r")) != NULL) {
        size = fread(mGyroCal, 1, STML0XX_GYRO_CAL_SIZE, fp);
        fclose(fp);
        if (size != STML0XX_GYRO_CAL_SIZE) {
            ALOGE("Gyro Cal file read failed");
            memset(mGyroCal, 0, sizeof(mGyroCal));
        } else {
            err = ioctl(dev_fd, STML0XX_IOCTL_SET_GYRO_CAL, mGyroCal);
            if (err < 0)
                ALOGE("Can't send Gyro Cal data");
        }
    }

    mGameRV = GameRotationVector::getInstance();
    mLAGravity = LinearAccelGravity::getInstance();
#endif

#ifdef _ENABLE_MAGNETOMETER
    mGeomagRV = GeoMagRotationVector::getInstance();
    mGeomagRVReady = 0;
    mRotationVect = RotationVector::getInstance();
#endif

    if ((fp = fopen(ACCEL_CAL_FILE, "r")) != NULL) {
        size = fread(mAccelCal, 1, STML0XX_ACCEL_CAL_SIZE, fp);
        fclose(fp);
        if (size != STML0XX_ACCEL_CAL_SIZE) {
            ALOGE("Accel Cal file read failed");
            memset(mAccelCal, 0, sizeof(mAccelCal));
        } else {
            err = ioctl(dev_fd, STML0XX_IOCTL_SET_ACCEL_CAL, mAccelCal);
            if (err < 0)
                ALOGE("Can't send Accel Cal data");
        }
    }

    if (!ioctl(dev_fd, STML0XX_IOCTL_GET_SENSORS, &flags16))  {
        mEnabled = flags16;
    }

    if (!ioctl(dev_fd, STML0XX_IOCTL_GET_WAKESENSORS, &flags24))  {
        mWakeEnabled = flags24;
    }
}

HubSensors::~HubSensors()
{
}

HubSensors *HubSensors::getInstance()
{
    return &self;
}

bool HubSensors::isHandleEnabled(uint64_t handle)
{
    return (mEnabledHandles & ((decltype(mEnabledHandles))1 << handle)) != 0;
}

int HubSensors::setEnable(int32_t handle, int en)
{
    int newState = en ? 1 : 0;
    uint32_t new_enabled;
    int found = 0;
    int err = 0;

    ALOGI("Sensorhub hal enable: %d - %d", handle, en);

    // Check non-wake sensors
    new_enabled = mEnabled;
    switch (handle) {
        case ID_A:
            mFusionSensors[ACCEL].enabled = newState;
            found = 1;
            break;
#ifdef _ENABLE_GYROSCOPE
        case ID_G:
            mFusionSensors[GYRO].enabled = newState;
            found = 1;
            break;
        case ID_UNCALIB_GYRO:
            mFusionSensors[UNCALIB_GYRO].enabled = newState;
            if (newState)
                new_enabled |= M_UNCALIB_GYRO;
            else
                new_enabled &= ~M_UNCALIB_GYRO;
            found = 1;
            break;
        case ID_GAME_RV:
            mFusionSensors[GAME_RV].enabled = newState;
            found = 1;
            break;
        case ID_LA:
            mFusionSensors[LINEAR_ACCEL].enabled = newState;
            found = 1;
            break;
        case ID_GRAVITY:
            mFusionSensors[GRAVITY].enabled = newState;
            found = 1;
            break;
#endif // _ENABLE_GYROSCOPE
        case ID_L:
            if (newState)
                new_enabled |= M_ALS;
            else
                new_enabled &= ~M_ALS;
            found = 1;
            break;
        case ID_DR:
            if (newState)
                new_enabled |= M_DISP_ROTATE;
            else
                new_enabled &= ~M_DISP_ROTATE;
            found = 1;
            break;
#ifdef _ENABLE_PEDO
        case ID_STEP_COUNTER:
            new_enabled &= ~M_STEP_COUNTER;
            if (newState)
                new_enabled |= M_STEP_COUNTER;
            found = 1;
            break;
        case ID_STEP_DETECTOR:
            new_enabled &= ~M_STEP_DETECTOR;
            if (newState)
                new_enabled |= M_STEP_DETECTOR;
            found = 1;
            break;
#endif
#ifdef _ENABLE_ACCEL_SECONDARY
        case ID_A2:
            if (newState)
                new_enabled |= M_ACCEL2;
            else
                new_enabled &= ~M_ACCEL2;
            found = 1;
            break;
#endif
#ifdef _ENABLE_MAGNETOMETER
        case ID_M:
            mFusionSensors[MAG].enabled = newState;
            found = 1;
            break;
        case ID_UM:
            mFusionSensors[UNCALIB_MAG].enabled = newState;
            found = 1;
            break;
        case ID_OR:
            mFusionSensors[ORIENTATION].enabled = newState;
            found = 1;
            break;
        case ID_GEOMAG_RV:
            mFusionSensors[GEOMAG_RV].enabled = newState;
            found = 1;
            break;
        case ID_RV:
            mFusionSensors[ROTATION_VECT].enabled = newState;
            found = 1;
            break;
#endif
    }

    if (found) {
        // Check if accel should be enabled or disabled
        if (isAccelNeeded()) {
            new_enabled |= M_ACCEL;
            err = updateAccelRate();
            ALOGE_IF(err, "Could not set accel rate(%s)", strerror(-err));
        } else {
            new_enabled &= ~M_ACCEL;
        }

#ifdef _ENABLE_GYROSCOPE
        // Check if gyro should be enabled or disabled
        if (isGyroNeeded()) {
            new_enabled |= M_GYRO;
            err = updateGyroRate();
            ALOGE_IF(err, "Could not set gyro rate(%s)", strerror(-err));
        } else {
            new_enabled &= ~M_GYRO;
        }
#endif

#ifdef _ENABLE_MAGNETOMETER
        // Check if magnetometer should be enabled or disabled
        if (isMagNeeded()) {
            new_enabled |= M_ECOMPASS;
            err = updateMagRate();
            ALOGE_IF(err, "Could not set mag rate(%s)", strerror(-err));
        } else {
            new_enabled &= ~M_ECOMPASS;
        }
#endif

        if (new_enabled != mEnabled) {
            err = ioctl(dev_fd, STML0XX_IOCTL_SET_SENSORS, &new_enabled);
            ALOGE_IF(err, "Could not change sensor state (%s)", strerror(-err));
            // Never return this error to the caller. This would result in a
            // failure to registerListener(), but regardless of failure, we
            // will consider these sensors 'registered' in the kernel driver.
            err = 0;
            mEnabled = new_enabled;
        }

#ifdef _ENABLE_MAGNETOMETER
        // If Geomag RV is disabled, reset the algo. This in turn will cause
        // the 9-axis RV to reset until mag samples have been received.
        // If only the 9-axis RV has been disabled, reset it directly
        if (!mFusionSensors[GEOMAG_RV].enabled) {
            mGeomagRV->processFusion(mFusionData, true);
        } else if (!mFusionSensors[ROTATION_VECT].enabled) {
            mRotationVect->processFusion(mFusionData, true);
        }
#endif
#ifdef _ENABLE_GYROSCOPE
        // If none of Game RV/Linear Accel/Gravity is enabled, reset the Game RV algo
        if (!mFusionSensors[GAME_RV].enabled &&
            !mFusionSensors[LINEAR_ACCEL].enabled &&
            !mFusionSensors[GRAVITY].enabled) {
            mGameRV->processFusion(mFusionData, true);
        }
#endif
    }

    // Check wake sensors
    new_enabled = mWakeEnabled;
    found = 0;
    switch (handle) {
        case ID_P:
            if (newState)
                new_enabled |= M_PROXIMITY;
            else
                new_enabled &= ~M_PROXIMITY;
            found = 1;
            break;
        case ID_FU:
            if (newState)
                new_enabled |= M_FLATUP;
            else
                new_enabled &= ~M_FLATUP;
            found = 1;
            break;
        case ID_FD:
            if (newState)
                new_enabled |= M_FLATDOWN;
            else
                new_enabled &= ~M_FLATDOWN;
            found = 1;
            break;
        case ID_S:
            if (newState)
                new_enabled |= M_STOWED;
            else
                new_enabled &= ~M_STOWED;
            found = 1;
            break;
        case ID_CA:
            if (newState)
                new_enabled |= M_CAMERA_GESTURE;
            else
                new_enabled &= ~M_CAMERA_GESTURE;
            found = 1;
            break;
#ifdef _ENABLE_LIFT
        case ID_LF:
            if (newState)
                new_enabled |= M_LIFT;
            else
                new_enabled &= ~M_LIFT;
            found = 1;
            break;
#endif
#ifdef _ENABLE_CHOPCHOP
        case ID_CC:
            if (newState)
                new_enabled |= M_CHOPCHOP;
            else
                new_enabled &= ~M_CHOPCHOP;
            found = 1;
            break;
#endif
        case ID_GLANCE_GESTURE:
            // enable only if moto_glance not enabled
            // disable only if moto_glance not enabled
            if (!isHandleEnabled(ID_MOTO_GLANCE_GESTURE)) {
                new_enabled &= ~M_GLANCE;
                if (newState)
                    new_enabled |= M_GLANCE;
                found = 1;
            }
            break;
        case ID_MOTO_GLANCE_GESTURE:
            // enable only if glance not enabled
            // disable only if glance not enabled
            if (!isHandleEnabled(ID_GLANCE_GESTURE)) {
                new_enabled &= ~M_GLANCE;
                if (newState)
                    new_enabled |= M_GLANCE;
                found = 1;
            }
            break;
        case ID_MOTION_DETECT:
            if (newState)
                new_enabled |= M_MOTION_DETECT;
            else
                new_enabled &= ~M_MOTION_DETECT;
            found = 1;
            break;
        case ID_STATIONARY_DETECT:
            if (newState)
                new_enabled |= M_STATIONARY_DETECT;
            else
                new_enabled &= ~M_STATIONARY_DETECT;
            found = 1;
            break;
    }

    if (found && (new_enabled != mWakeEnabled)) {
        err = ioctl(dev_fd, STML0XX_IOCTL_SET_WAKESENSORS, &new_enabled);
        ALOGE_IF(err, "Could not change wake sensor state (%s)", strerror(-err));
        // Never return this error to the caller. This would result in a
        // failure to registerListener(), but regardless of failure, we
        // will consider these sensors 'registered' in the kernel driver.
        err = 0;
        mWakeEnabled = new_enabled;
    }


    if (newState)
        mEnabledHandles |= ((decltype(mEnabledHandles))1 << handle);
    else
        mEnabledHandles &= ~((decltype(mEnabledHandles))1 << handle);

    return err;
}

int HubSensors::setDelay(int32_t handle, int64_t ns)
{
    int err = 0;
    unsigned int i;

    if (ns < 0)
        return -EINVAL;

    unsigned short delay = int64_t(ns) / 1000000;

    ALOGI("Sensorhub hal setDelay: %d - %d", handle, delay);

    // Clamp delay to min/max
    for (i = 0; i < sSensorList.size(); i++) {
        if ((SENSORS_HANDLE_BASE + handle) == sSensorList[i].handle) {
            delay = MAX(MIN(delay, sSensorList[i].maxDelay / 1000), sSensorList[i].minDelay / 1000);
            break;
        }
    }

    ALOGI("Sensorhub hal setdelay: %d - %d", handle, delay);

    switch (handle) {
        case ID_A:
            mFusionSensors[ACCEL].delay = delay;
            break;
#ifdef _ENABLE_GYROSCOPE
        case ID_G:
            mFusionSensors[GYRO].delay = delay;
            break;
        case ID_UNCALIB_GYRO:
            mFusionSensors[UNCALIB_GYRO].delay = delay;
            break;
        case ID_GAME_RV:
            mFusionSensors[GAME_RV].delay = delay;
            break;
        case ID_LA:
            mFusionSensors[LINEAR_ACCEL].delay = delay;
            break;
        case ID_GRAVITY:
            mFusionSensors[GRAVITY].delay = delay;
            break;
#endif
#ifdef _ENABLE_ACCEL_SECONDARY
        case ID_A2:
            err = ioctl(dev_fd, STML0XX_IOCTL_SET_ACC2_DELAY, &delay);
            break;
#endif
        case ID_L:
            err = ioctl(dev_fd, STML0XX_IOCTL_SET_ALS_DELAY, &delay);
            break;
        case ID_DR:
        case ID_P:
        case ID_FU:
        case ID_FD:
        case ID_S:
        case ID_CA:
        case ID_GLANCE_GESTURE:
        case ID_MOTO_GLANCE_GESTURE:
#ifdef _ENABLE_LIFT
        case ID_LF:
#endif
#ifdef _ENABLE_CHOPCHOP
        case ID_CC:
#endif
            break;
#ifdef _ENABLE_MAGNETOMETER
        case ID_M:
            mFusionSensors[MAG].delay = delay;
            break;
	case ID_UM:
            mFusionSensors[UNCALIB_MAG].delay = delay;
            break;
        case ID_OR:
            mFusionSensors[ORIENTATION].delay = delay;
            break;
        case ID_GEOMAG_RV:
            mFusionSensors[GEOMAG_RV].delay = delay;
            break;
        case ID_RV:
            mFusionSensors[ROTATION_VECT].delay = delay;
            break;
#endif
#ifdef _ENABLE_PEDO
        case ID_STEP_COUNTER:
            delay /= 1000; // convert to seconds for pedometer rate
            if (delay == 0)
                delay = 1;
            else if (delay > 3600) // 1 hour
                delay = 3600;

            err = ioctl(dev_fd, STML0XX_IOCTL_SET_STEP_COUNTER_DELAY, &delay);
            break;
        case ID_STEP_DETECTOR:
            err = 0;
            break;
#endif
        case ID_MOTION_DETECT:
            break;
        case ID_STATIONARY_DETECT:
            break;
        default:
            // Unsupported sensor
            ALOGE("HubSensors::setDelay - Unsupported sensor");
            err = -EINVAL;
            break;
    }

    err = updateAccelRate();
    ALOGE_IF(err, "Could not set accel rate(%s)", strerror(-err));

#ifdef _ENABLE_GYROSCOPE
    err = updateGyroRate();
    ALOGE_IF(err, "Could not set gyro rate(%s)", strerror(-err));
#endif

#ifdef _ENABLE_MAGNETOMETER
    err = updateMagRate();
    ALOGE_IF(err, "Could not set mag rate(%s)", strerror(-err));
#endif

    // Never return this error to the caller. This would result in a
    // failure to registerListener(), but regardless of failure, we
    // will consider these sensors 'registered' at the rate we tried
    // to write in the kernel driver.
    if( err == -EIO )
        err = 0;
    return err;
}

void HubSensors::logAlsEvent(int32_t lux, int64_t ts_ns) {
    static int32_t last_logged_val = -1;
    static int64_t last_logged_ts_ns;
    int32_t luxDelta = abs(lux - last_logged_val);
    if (last_logged_val == -1 ||
        (luxDelta > last_logged_val * 0.15 && luxDelta >= 5 &&
         ts_ns - last_logged_ts_ns >= 1000000000LL)) {
        ALOGD("ALS %d", lux);
        last_logged_val = lux;
        last_logged_ts_ns = ts_ns;
    }
}

int HubSensors::readEvents(sensors_event_t* d, int dLen)
{
    struct stml0xx_android_sensor_data buff;
    int ret;
    char timeBuf[32];
    struct tm* ptm = NULL;
    struct timeval timeutc;
    static long int sent_bug2go_sec = 0;
    sensors_event_t* data = d;
    FILE *fp;
    int size;

    // Ensure there are at least 4 slots free in the buffer
    // The following sensors populate multiple events per read:
    // DT_GYRO - upto 4 events
    // DT_GLANCE - upto 2 events
    sensors_event_t const* const dataEnd = d + dLen - 3;

    if (dLen < 1) {
        ALOGE("HubSensors::readEvents - bad length %d", dLen);
        return 0;
    }

    while (data < dataEnd && ((ret = read(data_fd, &buff, sizeof(struct stml0xx_android_sensor_data))) != 0)) {
        /* Sensorhub reset occurred, upload a bug2go if its been at least 10mins since previous bug2go*/
        /* remove this if-clause when corruption issue resolved */
        switch (buff.type) {
            case DT_FLUSH:
                data->version = META_DATA_VERSION;
                data->sensor = 0;
                data->type = SENSOR_TYPE_META_DATA;
                data->reserved0 = 0;
                data->timestamp = 0;
                data->meta_data.what = META_DATA_FLUSH_COMPLETE;
                data->meta_data.sensor = STM32TOH(buff.data + FLUSH);
                data++;
                break;
            case DT_ACCEL:
                mFusionData.accel.x = STM16TOH(buff.data+ACCEL_X) * CONVERT_A_X;
                mFusionData.accel.y = STM16TOH(buff.data+ACCEL_Y) * CONVERT_A_Y;
                mFusionData.accel.z = STM16TOH(buff.data+ACCEL_Z) * CONVERT_A_Z;
                mFusionData.accel.timestamp = buff.timestamp;
                if (mFusionSensors[ACCEL].enabled) {
                    data->version = SENSORS_EVENT_T_SIZE;
                    data->sensor = SENSORS_HANDLE_BASE + ID_A;
                    data->type = SENSOR_TYPE_ACCELEROMETER;
                    data->acceleration.x = mFusionData.accel.x;
                    data->acceleration.y = mFusionData.accel.y;
                    data->acceleration.z = mFusionData.accel.z;
                    data->acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
                    data->timestamp = mFusionData.accel.timestamp;
                    data++;
                }
#ifdef _ENABLE_MAGNETOMETER
                if (mFusionSensors[GEOMAG_RV].enabled || mFusionSensors[ROTATION_VECT].enabled) {
                    mGeomagRVReady = mGeomagRV->processFusion(mFusionData, false);
                    if (mFusionSensors[GEOMAG_RV].enabled) {
                        data->version = SENSORS_EVENT_T_SIZE;
                        data->sensor = SENSORS_HANDLE_BASE + ID_GEOMAG_RV;
                        data->type = SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR;
                        data->data[0] = mFusionData.geoMagRotation.a;
                        data->data[1] = mFusionData.geoMagRotation.b;
                        data->data[2] = mFusionData.geoMagRotation.c;
                        data->data[3] = mFusionData.geoMagRotation.d;
                        data->data[4] = mFusionData.geoMagRotation.accuracy;
                        data->timestamp = mFusionData.geoMagRotation.timestamp;
                        data++;
                    }
                }
#endif
                break;
#ifdef _ENABLE_ACCEL_SECONDARY
            case DT_ACCEL2:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = SENSORS_HANDLE_BASE + ID_A2;
                data->type = SENSOR_TYPE_ACCELEROMETER;
                data->acceleration.x = STM16TOH(buff.data+ACCEL_X) * CONVERT_A_X;
                data->acceleration.y = STM16TOH(buff.data+ACCEL_Y) * CONVERT_A_Y;
                data->acceleration.z = STM16TOH(buff.data+ACCEL_Z) * CONVERT_A_Z;
                data->acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
                data->timestamp = buff.timestamp;
                data++;
                break;
#endif
#ifdef _ENABLE_GYROSCOPE
            case DT_GYRO:
                mFusionData.gyro.x = STM16TOH(buff.data + GYRO_X) * CONVERT_G_P;
                mFusionData.gyro.y = STM16TOH(buff.data + GYRO_Y) * CONVERT_G_R;
                mFusionData.gyro.z = STM16TOH(buff.data + GYRO_Z) * CONVERT_G_Y;
                mFusionData.gyro.timestamp = buff.timestamp;
                if (mFusionSensors[GYRO].enabled) {
                    data->version = SENSORS_EVENT_T_SIZE;
                    data->sensor = SENSORS_HANDLE_BASE + ID_G;
                    data->type = SENSOR_TYPE_GYROSCOPE;
                    data->gyro.x = mFusionData.gyro.x;
                    data->gyro.y = mFusionData.gyro.y;
                    data->gyro.z = mFusionData.gyro.z;
                    data->gyro.status = SENSOR_STATUS_ACCURACY_HIGH;
                    data->timestamp = mFusionData.gyro.timestamp;
                    data++;
                }
                if (mFusionSensors[GAME_RV].enabled || mFusionSensors[LINEAR_ACCEL].enabled
                        || mFusionSensors[GRAVITY].enabled) {
                    mGameRV->processFusion(mFusionData, false);
                    if (mFusionSensors[GAME_RV].enabled) {
                        data->version = SENSORS_EVENT_T_SIZE;
                        data->sensor = SENSORS_HANDLE_BASE + ID_GAME_RV;
                        data->type = SENSOR_TYPE_GAME_ROTATION_VECTOR;
                        data->data[0] = mFusionData.gameRotation.a;
                        data->data[1] = mFusionData.gameRotation.b;
                        data->data[2] = mFusionData.gameRotation.c;
                        data->data[3] = mFusionData.gameRotation.d;
                        data->data[4] = mFusionData.gameRotation.accuracy;
                        data->timestamp = mFusionData.gameRotation.timestamp;
                        data++;
                    }
                    if (mFusionSensors[LINEAR_ACCEL].enabled || mFusionSensors[GRAVITY].enabled) {
                        mLAGravity->processFusion(mFusionData, false);

                        if (mFusionSensors[LINEAR_ACCEL].enabled) {
                            data->version = SENSORS_EVENT_T_SIZE;
                            data->sensor = SENSORS_HANDLE_BASE + ID_LA;
                            data->type = SENSOR_TYPE_LINEAR_ACCELERATION;
                            data->acceleration.x = mFusionData.linearAccel.x;
                            data->acceleration.y = mFusionData.linearAccel.y;
                            data->acceleration.z = mFusionData.linearAccel.z;
                            data->acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
                            data->timestamp = mFusionData.linearAccel.timestamp;
                            data++;
                        }
                        if (mFusionSensors[GRAVITY].enabled) {
                            data->version = SENSORS_EVENT_T_SIZE;
                            data->sensor = SENSORS_HANDLE_BASE + ID_GRAVITY;
                            data->type = SENSOR_TYPE_GRAVITY;
                            data->acceleration.x = mFusionData.gravity.x;
                            data->acceleration.y = mFusionData.gravity.y;
                            data->acceleration.z = mFusionData.gravity.z;
                            data->acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
                            data->timestamp = mFusionData.gravity.timestamp;
                            data++;
                        }
                    }
                }
#ifdef _ENABLE_MAGNETOMETER
                if (mFusionSensors[ROTATION_VECT].enabled) {
                    mRotationVect->processFusion(mFusionData, !mGeomagRVReady);

                    data->version = SENSORS_EVENT_T_SIZE;
                    data->sensor = SENSORS_HANDLE_BASE + ID_RV;
                    data->type = SENSOR_TYPE_ROTATION_VECTOR;
                    data->data[0] = mFusionData.rotationVector.a;
                    data->data[1] = mFusionData.rotationVector.b;
                    data->data[2] = mFusionData.rotationVector.c;
                    data->data[3] = mFusionData.rotationVector.d;
                    data->data[4] = mFusionData.rotationVector.accuracy;
                    data->timestamp = mFusionData.rotationVector.timestamp;
                    data++;
                }
#endif
                break;
            case DT_UNCALIB_GYRO:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor =  SENSORS_HANDLE_BASE + ID_UNCALIB_GYRO;
                data->type = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED;
                data->uncalibrated_gyro.x_uncalib = STM16TOH(buff.data + UNCALIB_GYRO_X) * CONVERT_G_P;
                data->uncalibrated_gyro.y_uncalib = STM16TOH(buff.data + UNCALIB_GYRO_Y) * CONVERT_G_R;
                data->uncalibrated_gyro.z_uncalib = STM16TOH(buff.data + UNCALIB_GYRO_Z) * CONVERT_G_Y;
                data->uncalibrated_gyro.x_bias = STM16TOH(buff.data + UNCALIB_GYRO_X_BIAS) * CONVERT_BIAS_G_P;
                data->uncalibrated_gyro.y_bias = STM16TOH(buff.data + UNCALIB_GYRO_Y_BIAS) * CONVERT_BIAS_G_R;
                data->uncalibrated_gyro.z_bias = STM16TOH(buff.data + UNCALIB_GYRO_Z_BIAS) * CONVERT_BIAS_G_Y;
                data->timestamp = buff.timestamp;
                data++;
                break;
#endif
            case DT_ALS:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = SENSORS_HANDLE_BASE + ID_L;
                data->type = SENSOR_TYPE_LIGHT;
                data->light = (uint16_t)STM16TOH(buff.data + LIGHT_LIGHT);
                data->timestamp = buff.timestamp;
                logAlsEvent(data->light, data->timestamp);
                data++;
                break;
            case DT_DISP_ROTATE:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = SENSORS_HANDLE_BASE + ID_DR;
                data->type = SENSOR_TYPE_DISPLAY_ROTATE;
                if (buff.data[ROTATE_ROTATE] == DISP_FLAT)
                    data->data[0] = DISP_UNKNOWN;
                else
                    data->data[0] = buff.data[ROTATE_ROTATE];

                data->timestamp = buff.timestamp;
                data++;
                break;
            case DT_PROX:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = SENSORS_HANDLE_BASE + ID_P;
                data->type = SENSOR_TYPE_PROXIMITY;
                if (buff.data[PROXIMITY_PROXIMITY] == 0) {
                    data->distance = PROX_UNCOVERED;
                    ALOGE("Proximity uncovered");
                } else if (buff.data[PROXIMITY_PROXIMITY] == 1) {
                    data->distance = PROX_COVERED;
                    ALOGE("Proximity covered 1");
                } else {
                    data->distance = PROX_SATURATED;
                    ALOGE("Proximity covered 2");
                }
                data->timestamp = buff.timestamp;
                data++;
                break;
            case DT_FLAT_UP:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = SENSORS_HANDLE_BASE + ID_FU;
                data->type = SENSOR_TYPE_FLAT_UP;
                if (buff.data[FLAT_FLAT] == 0x01)
                    data->data[0] = FLAT_DETECTED;
                else
                    data->data[0] = FLAT_NOTDETECTED;
                data->timestamp = buff.timestamp;
                data++;
                break;
            case DT_FLAT_DOWN:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = SENSORS_HANDLE_BASE + ID_FD;
                data->type = SENSOR_TYPE_FLAT_DOWN;
                if (buff.data[FLAT_FLAT] == 0x02)
                    data->data[0] = FLAT_DETECTED;
                else
                    data->data[0] = FLAT_NOTDETECTED;
                data->timestamp = buff.timestamp;
                data++;
                break;
            case DT_STOWED:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = SENSORS_HANDLE_BASE + ID_S;
                data->type = SENSOR_TYPE_STOWED;
                data->data[0] = buff.data[STOWED_STOWED];
                data->timestamp = buff.timestamp;
                data++;
                break;
            case DT_CAMERA_ACT:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = SENSORS_HANDLE_BASE + ID_CA;
                data->type = SENSOR_TYPE_CAMERA_ACTIVATE;
                data->data[0] = STML0XX_CAMERA_DATA;
                data->data[1] = STM16TOH(buff.data + CAMERA_CAMERA);
                data->timestamp = buff.timestamp;
                data++;
                break;
#ifdef _ENABLE_GYROSCOPE
            case DT_GYRO_CAL:
                ret = ioctl(dev_fd, STML0XX_IOCTL_GET_GYRO_CAL, mGyroCal);
                if (ret < 0) {
                    ALOGE("Can't read Gyro Cal data");
                } else {
                    if ((fp = fopen(GYRO_CAL_FILE, "w")) == NULL) {
                        ALOGE("Can't open Gyro Cal file");
                    } else {
                        size = fwrite(mGyroCal, 1, STML0XX_GYRO_CAL_SIZE, fp);
                        fclose(fp);
                        if (size != STML0XX_GYRO_CAL_SIZE)
                            ALOGE("Error writing Gyro Cal file");
                    }
                }
                break;
#endif
            case DT_ACCEL_CAL:
                ret = ioctl(dev_fd, STML0XX_IOCTL_GET_ACCEL_CAL, mAccelCal);
                if (ret < 0) {
                    ALOGE("Can't read Accel Cal data");
                } else {
                    if ((fp = fopen(ACCEL_CAL_FILE, "w")) == NULL) {
                        ALOGE("Can't open Accel Cal file");
                    } else {
                        size = fwrite(mAccelCal, 1, STML0XX_ACCEL_CAL_SIZE, fp);
                        fclose(fp);
                        if (size != STML0XX_ACCEL_CAL_SIZE)
                            ALOGE("Error writing Accel Cal file");
                    }
                }
                break;
            case DT_RESET:
                time(&timeutc.tv_sec);
                if (buff.data[0] <= RESET_REASON_MAX_CODE)
                    mErrorCnt[buff.data[0]]++;
                if ((sent_bug2go_sec == 0) ||
                    (timeutc.tv_sec - sent_bug2go_sec > 24*60*60)) {
                    // put timestamp in dropbox file
                    ptm = localtime(&(timeutc.tv_sec));
                    if (ptm != NULL) {
                        strftime(timeBuf, sizeof(timeBuf), "%m-%d %H:%M:%S", ptm);
                        capture_dump(timeBuf, buff.type, SENSORHUB_DUMPFILE,
                            DROPBOX_FLAG_TEXT | DROPBOX_FLAG_GZIP);
                    }
                    sent_bug2go_sec = timeutc.tv_sec;
                } else {
                    mPendingBug2go = 1;
                }
                break;
#ifdef _ENABLE_LIFT
            case DT_LIFT:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = SENSORS_HANDLE_BASE + ID_LF;
                data->type = SENSOR_TYPE_LIFT_GESTURE;
                data->data[0] = STM32TOH(buff.data + LIFT_DISTANCE);
                data->data[1] = STM32TOH(buff.data + LIFT_ROTATION);
                data->data[2] = STM32TOH(buff.data + LIFT_GRAV_DIFF);
                data->timestamp = buff.timestamp;
                data++;
                break;
#endif
#ifdef _ENABLE_CHOPCHOP
            case DT_CHOPCHOP:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = SENSORS_HANDLE_BASE + ID_CC;
                data->type = SENSOR_TYPE_CHOPCHOP_GESTURE;
                data->data[0] = STM16TOH(buff.data + CHOPCHOP_CHOPCHOP);
                data->timestamp = buff.timestamp;
                data++;
                break;
#endif
#ifdef _ENABLE_PEDO
            case DT_STEP_COUNTER:
            {
                static uint32_t last_stepcount;
                static uint32_t step_offset;
                uint32_t stepcount;

                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor =  SENSORS_HANDLE_BASE + ID_STEP_COUNTER;
                data->type = SENSOR_TYPE_STEP_COUNTER;
                /* data from sensors sent as 4 bytes. Thats plenty of steps */
                stepcount = (buff.data[0] << 24 |
                             buff.data[1] << 16 |
                             buff.data[2] << 8  |
                             buff.data[3]);
                if(stepcount + step_offset < last_stepcount)
                {
                    /* hub reset, determine offset and apply, so users
                       only see contiguous steps */
                    step_offset = last_stepcount;
                    ALOGD("Saving %d footsteps", step_offset);
                }
                last_stepcount = stepcount + step_offset;

                data->u64.step_counter = (uint64_t)(last_stepcount);
                data->timestamp = buff.timestamp;
                data++;
                break;
            }
            case DT_STEP_DETECTOR:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor =  SENSORS_HANDLE_BASE + ID_STEP_DETECTOR;
                data->type = SENSOR_TYPE_STEP_DETECTOR;
                data->data[0] = (uint16_t)buff.data[0];
                data->timestamp = buff.timestamp;
                data++;
                break;
#endif
            case DT_GLANCE:
                if (isHandleEnabled(ID_MOTO_GLANCE_GESTURE)) {
                    data->version = SENSORS_EVENT_T_SIZE;
                    data->sensor = ID_MOTO_GLANCE_GESTURE;
                    data->type = SENSOR_TYPE_MOTO_GLANCE_GESTURE;
                    data->data[0] = STM16TOH(buff.data); /* Gesture that triggered glance */
                    data->data[1] = 0;
                    data->data[2] = 0;
                    data->timestamp = buff.timestamp;
                    data++;
                }

                if (isHandleEnabled(ID_GLANCE_GESTURE)) {
                    data->version = SENSORS_EVENT_T_SIZE;
                    data->sensor = ID_GLANCE_GESTURE;
                    data->type = SENSOR_TYPE_GLANCE_GESTURE;
                    data->data[0] = 1.0;                   /* set to 1 for Android compatibility */
                    data->data[1] = STM16TOH(buff.data);   /* Currently blocked by Android FW */
                    data->data[2] = 0;
                    data->timestamp = buff.timestamp;
                    data++;
                }
#ifdef _ENABLE_MAGNETOMETER
            case DT_MAG:
                mFusionData.mag.x = STM16TOH(buff.data + MAGNETIC_X) * CONVERT_M_X;
                mFusionData.mag.y = STM16TOH(buff.data + MAGNETIC_Y) * CONVERT_M_Y;
                mFusionData.mag.z = STM16TOH(buff.data + MAGNETIC_Z) * CONVERT_M_Z;
                mFusionData.mag.timestamp = buff.timestamp;
                if (mFusionSensors[MAG].enabled) {
                    data->version = SENSORS_EVENT_T_SIZE;
                    data->sensor = SENSORS_HANDLE_BASE + ID_M;
                    data->type = SENSOR_TYPE_MAGNETIC_FIELD;
                    data->magnetic.x = mFusionData.mag.x;
                    data->magnetic.y = mFusionData.mag.y;
                    data->magnetic.z = mFusionData.mag.z;
                    data->magnetic.status = buff.status;
                    data->timestamp = mFusionData.mag.timestamp;
                    data++;
                }
                break;
            case DT_UNCALIB_MAG:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor =  SENSORS_HANDLE_BASE + ID_UM;
                data->type = SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED;
                data->uncalibrated_magnetic.x_uncalib = STM16TOH(buff.data + UNCALIB_MAGNETIC_X) * CONVERT_M_X;
                data->uncalibrated_magnetic.y_uncalib = STM16TOH(buff.data + UNCALIB_MAGNETIC_Y) * CONVERT_M_Y;
                data->uncalibrated_magnetic.z_uncalib = STM16TOH(buff.data + UNCALIB_MAGNETIC_Z) * CONVERT_M_Z;
                data->uncalibrated_magnetic.x_bias = STM16TOH(buff.data + UNCALIB_MAGNETIC_X_BIAS) * CONVERT_BIAS_M_X;
                data->uncalibrated_magnetic.y_bias = STM16TOH(buff.data + UNCALIB_MAGNETIC_Y_BIAS) * CONVERT_BIAS_M_Y;
                data->uncalibrated_magnetic.z_bias = STM16TOH(buff.data + UNCALIB_MAGNETIC_Z_BIAS) * CONVERT_BIAS_M_Z;
                data->timestamp = buff.timestamp;
                data++;
                break;
            case DT_ORIENT:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor =  SENSORS_HANDLE_BASE + ID_OR;
                data->type = SENSOR_TYPE_ORIENTATION;
                data->orientation.azimuth = STM16TOH(buff.data + ORIENTATION_AZIMUTH) * CONVERT_O_Y;
                data->orientation.pitch = STM16TOH(buff.data + ORIENTATION_PITCH) * CONVERT_O_P;
                data->orientation.roll = STM16TOH(buff.data + ORIENTATION_ROLL) * CONVERT_O_R;
                data->orientation.status = buff.status;
                data->timestamp = buff.timestamp;
                data++;
                break;
#endif
            case DT_MOTION_DETECT:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor =  SENSORS_HANDLE_BASE + ID_MOTION_DETECT;
                data->type = SENSOR_TYPE_MOTION_DETECT;
                data->data[0] = 1.0;
                data->timestamp = buff.timestamp;
                data++;
                /* Disable, because this is a one shot sensor */
                setEnable(ID_MOTION_DETECT, 0);
                break;
            case DT_STATIONARY_DETECT:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor =  SENSORS_HANDLE_BASE + ID_STATIONARY_DETECT;
                data->type = SENSOR_TYPE_STATIONARY_DETECT;
                data->data[0] = 1.0;
                data->timestamp = buff.timestamp;
                data++;
                /* Disable, because this is a one shot sensor */
                setEnable(ID_STATIONARY_DETECT, 0);
                break;
            default:
                break;
        }
    }
    if (mPendingBug2go == 1) {
        time(&timeutc.tv_sec);
        if (timeutc.tv_sec - sent_bug2go_sec > 24*60*60) {
            // put timestamp in dropbox file
            ptm = localtime(&(timeutc.tv_sec));
            if (ptm != NULL) {
                strftime(timeBuf, sizeof(timeBuf), "%m-%d %H:%M:%S", ptm);
                capture_dump(timeBuf, buff.type, SENSORHUB_DUMPFILE,
                    DROPBOX_FLAG_TEXT | DROPBOX_FLAG_GZIP);
            }
            sent_bug2go_sec = timeutc.tv_sec;
            mPendingBug2go = 0;
        }
    }

    return data - d;
}

int HubSensors::flush(int32_t handle)
{
    int ret = 0;
    if (handle > MIN_SENSOR_ID && handle < MAX_SENSOR_ID) {
        ret = ioctl(dev_fd,  STML0XX_IOCTL_SET_FLUSH, &handle);
    }
    return ret;
}

gzFile HubSensors::open_dropbox_file(const char* timestamp, const char* dst, const int flags)
{
    (void)dst;

    char dropbox_path[128];
    pid_t pid = getpid();

    snprintf(dropbox_path, sizeof(dropbox_path), "%s/%s:%d:%u-%s",
             DROPBOX_DIR, DROPBOX_TAG, flags, pid, timestamp);
    ALOGD("stml0xx - dumping to dropbox file[%s]...\n", dropbox_path);

    return gzopen(dropbox_path, "wb");
}

short HubSensors::capture_dump(char* timestamp, const int id, const char* dst, const int flags)
{
    char buffer[COPYSIZE] = {0};
    int rc = 0, i = 0;
    gzFile dropbox_file = NULL;

    dropbox_file = open_dropbox_file(timestamp, dst, flags);
    if(dropbox_file == NULL) {
        ALOGE("ERROR! unable to open dropbox file[errno:%d(%s)]\n", errno, strerror(errno));
    } else {
        // put timestamp in dropbox file
        rc = snprintf(buffer, COPYSIZE, "timestamp:%s\n", timestamp);
        gzwrite(dropbox_file, buffer, rc);
        rc = snprintf(buffer, COPYSIZE, "reason:%02d\n", id);
        gzwrite(dropbox_file, buffer, rc);

        for (i = 0; i <= RESET_REASON_MAX_CODE; i++) {
            rc = snprintf(buffer, COPYSIZE, "[%d]:%d\n", i, mErrorCnt[i]);
            gzwrite(dropbox_file, buffer, rc);
        }
        memset(mErrorCnt, 0, sizeof(mErrorCnt));

        gzclose(dropbox_file);
        // to commit buffer cache to disk
        sync();
    }

    return 0;
}

#ifdef _ENABLE_GYROSCOPE
int HubSensors::updateGyroRate()
{
    static unsigned short prev_delay;
    unsigned short delay = GYRO_MAX_DELAY_US / 1000; // convert to ms
    int sensorIdx, err = 0;

    // Find the shortest delay from all enabled sensors that require gyro
    for (sensorIdx = 0; sensorIdx < NUM_FUSION_DEVICES; sensorIdx++) {
        if (mFusionSensors[sensorIdx].usesGyro &&
            mFusionSensors[sensorIdx].enabled &&
            mFusionSensors[sensorIdx].delay < delay) {
            delay = isRotationVectorRunning() ?
                    MIN(mFusionSensors[sensorIdx].delay, FUSION_MAX_DELAY_US / 1000) :
                    mFusionSensors[sensorIdx].delay;
        }
    }
    delay = MAX(delay, GYRO_MIN_DELAY_US / 1000);

    if (delay != prev_delay) {
        // Update gyro rate
        err = ioctl(dev_fd, STML0XX_IOCTL_SET_GYRO_DELAY, &delay);
        ALOGI("HubSensors::updateGyroRate %d", delay);
        prev_delay = delay;
    }
    return err;
}

bool HubSensors::isGyroNeeded()
{
    int sensorIdx;
    for (sensorIdx = 0; sensorIdx < NUM_FUSION_DEVICES; sensorIdx++) {
        if (mFusionSensors[sensorIdx].usesGyro &&
            mFusionSensors[sensorIdx].enabled)
            return true;
    }
    return false;
}
#endif

#ifdef _ENABLE_MAGNETOMETER
int HubSensors::updateMagRate()
{
    static unsigned short prev_delay;
    unsigned short delay = MAG_MAX_DELAY_US / 1000; // convert to ms
    int sensorIdx, err = 0;

    // Find the shortest delay from all enabled sensors that require mag
    for (sensorIdx = 0; sensorIdx < NUM_FUSION_DEVICES; sensorIdx++) {
        if (mFusionSensors[sensorIdx].usesMag &&
            mFusionSensors[sensorIdx].enabled &&
            mFusionSensors[sensorIdx].delay < delay) {
            delay = isRotationVectorRunning() ?
                    MIN(mFusionSensors[sensorIdx].delay, FUSION_MAX_DELAY_US / 1000) :
                    mFusionSensors[sensorIdx].delay;
        }
    }
    delay = MAX(delay, MAG_MIN_DELAY_US / 1000);

    if (delay != prev_delay) {
        // Update mag rate
        err = ioctl(dev_fd, STML0XX_IOCTL_SET_MAG_DELAY, &delay);
        ALOGI("HubSensors::updateMagRate %d", delay);
        prev_delay = delay;
    }
    return err;
}

bool HubSensors::isMagNeeded()
{
    int sensorIdx;
    for (sensorIdx = 0; sensorIdx < NUM_FUSION_DEVICES; sensorIdx++) {
        if (mFusionSensors[sensorIdx].usesMag &&
            mFusionSensors[sensorIdx].enabled)
            return true;
    }
    return false;
}
#endif

int HubSensors::updateAccelRate()
{
    static unsigned short prev_delay;
    unsigned short delay = ACCEL_MAX_DELAY_US / 1000; // convert to ms
    int sensorIdx, err = 0;

    // Find the shortest delay from all enabled sensors that require accel
    for (sensorIdx = 0; sensorIdx < NUM_FUSION_DEVICES; sensorIdx++) {
        if (mFusionSensors[sensorIdx].usesAccel &&
            mFusionSensors[sensorIdx].enabled &&
            mFusionSensors[sensorIdx].delay < delay) {
            delay = isRotationVectorRunning() ?
                    MIN(mFusionSensors[sensorIdx].delay, FUSION_MAX_DELAY_US / 1000) :
                    mFusionSensors[sensorIdx].delay;
        }
    }
    delay = MAX(delay, ACCEL_MIN_DELAY_US / 1000);

    if (delay != prev_delay) {
        // Update accel rate
        err = ioctl(dev_fd, STML0XX_IOCTL_SET_ACC_DELAY, &delay);
        ALOGI("HubSensors::updateAccelRate %d", delay);
        prev_delay = delay;
    }
    return err;
}

bool HubSensors::isAccelNeeded()
{
    for (int sensorIdx = 0; sensorIdx < NUM_FUSION_DEVICES; sensorIdx++) {
        if (mFusionSensors[sensorIdx].usesAccel &&
            mFusionSensors[sensorIdx].enabled)
            return true;
    }
    return false;
}

bool HubSensors::isRotationVectorRunning()
{
    bool rvRunning = (
        false
#ifdef _ENABLE_MAGNETOMETER
        || mFusionSensors[GEOMAG_RV].enabled
        || mFusionSensors[ROTATION_VECT].enabled
#endif
#ifdef _ENABLE_GYROSCOPE
        || mFusionSensors[GAME_RV].enabled
        || mFusionSensors[LINEAR_ACCEL].enabled
        || mFusionSensors[GRAVITY].enabled
#endif
    );
    return rvRunning;
}

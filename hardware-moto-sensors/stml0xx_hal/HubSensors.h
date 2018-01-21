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
 * Copyright (C) 2015 Motorola Mobility LLC
 */

#ifndef ANDROID_HUB_SENSOR_H
#define ANDROID_HUB_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <endian.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <zlib.h>
#include <time.h>
#include <private/android_filesystem_config.h>

#include <linux/stml0xx.h>

#include "FusionSensorBase.h"
#include "GameRotationVector.h"
#include "LinearAccelGravity.h"
#include "SensorBase.h"
#include "SensorList.h"
#include "Sensors.h"

#ifdef _ENABLE_MAGNETOMETER
#include "GeoMagRotationVector.h"
#include "RotationVector.h"
#endif

/*****************************************************************************/

#define SENSORHUB_DEVICE_NAME       "/dev/stml0xx"
#define SENSORHUB_AS_DATA_NAME      "/dev/stml0xx_as"

#define SENSORS_EVENT_T_SIZE sizeof(sensors_event_t);
#define DROPBOX_DIR "/data/system/dropbox-add"
#define DROPBOX_TAG "SENSOR_HUB"
#define SENSORHUB_DUMPFILE  "sensor_hub"
#define DROPBOX_FLAG_TEXT        2
#define DROPBOX_FLAG_GZIP        4
#define COPYSIZE 256

#define GYRO_CAL_FILE  "/data/misc/sensorhub/gyro_cal.bin"
#define ACCEL_CAL_FILE "/data/misc/sensorhub/accel_cal.bin"

// Defines for offsets into the sensorhub event data.
#define ACCEL_X (0 * sizeof(int16_t))
#define ACCEL_Y (1 * sizeof(int16_t))
#define ACCEL_Z (2 * sizeof(int16_t))
#define GYRO_X (0 * sizeof(int16_t))
#define GYRO_Y (1 * sizeof(int16_t))
#define GYRO_Z (2 * sizeof(int16_t))
#define UNCALIB_GYRO_X (0 * sizeof(int16_t))
#define UNCALIB_GYRO_Y (1 * sizeof(int16_t))
#define UNCALIB_GYRO_Z (2 * sizeof(int16_t))
#define UNCALIB_GYRO_X_BIAS (3 * sizeof(int16_t))
#define UNCALIB_GYRO_Y_BIAS (4 * sizeof(int16_t))
#define UNCALIB_GYRO_Z_BIAS (5 * sizeof(int16_t))
#define LIGHT_LIGHT (0 * sizeof(int16_t))
#define ROTATE_ROTATE (0 * sizeof(int8_t))
#define PROXIMITY_PROXIMITY (0 * sizeof(int8_t))
#define FLAT_FLAT (0 * sizeof(int8_t))
#define STOWED_STOWED (0 * sizeof(int8_t))
#define CAMERA_CAMERA (0 * sizeof(int16_t))
#define FLUSH (0 * sizeof(int32_t))
#define LIFT_DISTANCE (0 * sizeof(int8_t))
#define LIFT_ROTATION (4 * sizeof(int8_t))
#define LIFT_GRAV_DIFF (8 * sizeof(int8_t))
#define CHOPCHOP_CHOPCHOP (0 * sizeof(int8_t))
#define MAGNETIC_X (0 * sizeof(int16_t))
#define MAGNETIC_Y (1 * sizeof(int16_t))
#define MAGNETIC_Z (2 * sizeof(int16_t))
#define UNCALIB_MAGNETIC_X (0 * sizeof(int16_t))
#define UNCALIB_MAGNETIC_Y (1 * sizeof(int16_t))
#define UNCALIB_MAGNETIC_Z (2 * sizeof(int16_t))
#define UNCALIB_MAGNETIC_X_BIAS (3 * sizeof(int16_t))
#define UNCALIB_MAGNETIC_Y_BIAS (4 * sizeof(int16_t))
#define UNCALIB_MAGNETIC_Z_BIAS (5 * sizeof(int16_t))
#define ORIENTATION_AZIMUTH (0 * sizeof(int16_t))
#define ORIENTATION_PITCH   (1 * sizeof(int16_t))
#define ORIENTATION_ROLL    (2 * sizeof(int16_t))

#define STM16TOH(p) (int16_t) be16toh(*((uint16_t *) (p)))
#define STM32TOH(p) (int32_t) be32toh(*((uint32_t *) (p)))

struct input_event;

class HubSensors : public SensorBase {
public:
    HubSensors();
    virtual ~HubSensors();

    virtual bool isHandleEnabled(uint64_t handle);
    virtual int setEnable(int32_t handle, int enabled) override;
    virtual int setDelay(int32_t handle, int64_t ns) override;
    virtual int readEvents(sensors_event_t* data, int count) override;
    virtual int flush(int32_t handle) override;

    static HubSensors* getInstance();

private:
    enum fusion_enum
    {
        ACCEL,
#ifdef _ENABLE_GYROSCOPE
        GYRO,
        UNCALIB_GYRO,
        GAME_RV,
        LINEAR_ACCEL,
        GRAVITY,
#endif
#ifdef _ENABLE_MAGNETOMETER
        MAG,
        UNCALIB_MAG,
        ORIENTATION,
        GEOMAG_RV,
        ROTATION_VECT,
#endif
        NUM_FUSION_DEVICES
    };

    typedef struct {
        bool enabled;
        bool usesAccel;
        bool usesGyro;
        bool usesMag;
        unsigned short delay; // ms
    } FusionSensor;

    FusionSensor mFusionSensors[NUM_FUSION_DEVICES];

    static HubSensors self;
    uint32_t mEnabled;
    uint32_t mWakeEnabled;
    uint32_t mPendingMask;
    uint32_t mFlushEnabled;
    uint64_t mEnabledHandles;
    uint32_t mPendingBug2go;
    FusionData mFusionData;

#ifdef _ENABLE_GYROSCOPE
    //! \brief gyroscope calibration table
    uint8_t mGyroCal[STML0XX_GYRO_CAL_SIZE];
    GameRotationVector *mGameRV;
    LinearAccelGravity *mLAGravity;
#endif

#ifdef _ENABLE_MAGNETOMETER
    GeoMagRotationVector *mGeomagRV;
    uint8_t mGeomagRVReady;
    RotationVector *mRotationVect;
#endif

    uint8_t mAccelCal[STML0XX_ACCEL_CAL_SIZE];

    uint8_t mErrorCnt[RESET_REASON_MAX_CODE + 1];
    gzFile open_dropbox_file(const char* timestamp, const char* dst, const int flags);
    short capture_dump(char* timestamp, const int id, const char* dst, const int flags);
    void logAlsEvent(int32_t lux, int64_t ts_ns);
    bool isRotationVectorRunning(void);

#ifdef _ENABLE_GYROSCOPE
    /*!
     * \brief Helper to update gyro rate
     *
     * Determines the gyro rate based on the current registrations
     * of one or more sensors that involve sensor fusion
     *
     * \returns ioctl() status resulting from gyro rate set
     */
    int updateGyroRate();
    bool isGyroNeeded();
#endif
#ifdef _ENABLE_MAGNETOMETER
    /*!
     * \brief Helper to update mag rate
     *
     * Determines the mag rate based on the current registrations
     * of one or more sensors that involve sensor fusion
     *
     * \returns ioctl() status resulting from mag rate set
     */
    int updateMagRate();
    bool isMagNeeded();
#endif
    /*!
     * \brief Helper to update accel rate
     *
     * Determines the accel rate based on the current registrations
     * of one or more sensors that involve sensor fusion
     *
     * \returns ioctl() status resulting from accel rate set
     */
    int updateAccelRate();
    bool isAccelNeeded();
};

/*****************************************************************************/

#endif  // ANDROID_HUB_SENSOR_H

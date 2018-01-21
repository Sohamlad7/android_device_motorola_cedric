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

#include <float.h>
#include <limits.h>
#include <string.h>

#include <hardware/sensors.h>
#include <hardware/mot_sensorhub_stml0xx.h>

#include "Sensors.h"
#include "SensorList.h"

/*****************************************************************************/

/*
 * The SENSORS Module
 */

/* Accel/Gyro conditional compilation choices */
#ifdef _ENABLE_BMI160
#define VENDOR_ACCEL  "Bosch"
#define VENDOR_GYRO   "Bosch"
/* Current draw figures in mA */
#define ACCEL_MA      0.55f
#define GYRO_MA       0.90f
#endif

#if _ENABLE_KXCJ9
#define VENDOR_ACCEL  "Kionix"
/* Current draw figures in mA */
#define ACCEL_MA      0.12f
#endif

/* Vendor names */
#define VENDOR_MAG     "Asahi Kasei"
#define VENDOR_MOT     "Motorola"
#define VENDOR_PROXALS "TAOS"

/* Range settings */
#define ACCEL_FULLSCALE_G  RANGE_G
#define GYRO_FULLSCALE_DPS (2000.f)
#define MAG_FULLSCALE_UT   (4912.f)
#define ALS_FULLSCALE_LUX  ((float)UINT16_MAX)

/* Resolution settings */
#define GYRO_QUANTIZATION_LEVELS  (INT16_MAX)
#define ALS_QUANTIZATION_LEVELS   (UINT16_MAX)
#define RV_QUANTIZATION_LEVELS    (INT16_MAX)
#define GRAV_QUANTIZATION_LEVELS  (INT16_MAX)

/* Min delays */
//ACCEL_MIN_DELAY_US is defined in SensorList.h
//GYRO_MIN_DELAY_US is defined in SensorList.h
//MAG_MIN_DELAY_US is defined in SensorList.h
#define STEP_MIN_DELAY_US  1000000 /* 1 sec */

/* Max delays */
//ACCEL_MAX_DELAY_US is defined in SensorList.h
//GYRO_MAX_DELAY_US is defined in SensorList.h
//MAG_MAX_DELAY_US is defined in SensorList.h
//FUSION_MAX_DELAY_US is defined in SensorList.h
#define STEP_MAX_DELAY_US   0

/* Various current draw figures in mA */
#define MAG_MA    0.10f
#define ALS_MA    0.25f
#define PROX_MA   0.0467f /* 100mA (LED drive) * (7us/pulse * 4 pulses)/60ms */

/* Estimated algorithm current draw in mA*/
#define ORIENT_ALGO_MA    1.0f
#define CAM_ACT_ALGO_MA   1.0f
#define CHOP_CHOP_ALGO_MA 1.0f
#define LIFT_ALGO_MA      1.0f
#define MOT_GAMERV_MA     1.0f
#define MOT_LAGRAV_MA     1.0f
#define GLANCE_ALGO_MA    (ACCEL_MA + PROX_MA + ALS_MA)

std::vector<struct sensor_t> sSensorList = {
    { .name = "3-axis Accelerometer",
        .vendor = VENDOR_ACCEL,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_A,
        .type = SENSOR_TYPE_ACCELEROMETER,
        .maxRange = ACCEL_FULLSCALE_G * GRAVITY_EARTH,
        .resolution = GRAVITY_EARTH / LSG,
        .power = ACCEL_MA,
        .minDelay = ACCEL_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_ACCELEROMETER,
        .requiredPermission = "",
        .maxDelay = ACCEL_MAX_DELAY_US,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} },
#ifdef _ENABLE_GYROSCOPE
    { .name = "3-axis Gyroscope",
        .vendor = VENDOR_GYRO,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_G,
        .type = SENSOR_TYPE_GYROSCOPE,
        .maxRange = GYRO_FULLSCALE_DPS,
        .resolution = GYRO_FULLSCALE_DPS / GYRO_QUANTIZATION_LEVELS,
        .power = GYRO_MA,
        .minDelay = GYRO_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_GYROSCOPE,
        .requiredPermission = SENSOR_STRING_TYPE_GYROSCOPE,
        .maxDelay = GYRO_MAX_DELAY_US,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} },
    { .name = "3-axis Uncalibrated Gyroscope",
        .vendor = VENDOR_GYRO,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_UNCALIB_GYRO,
        .type = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED,
        .maxRange = GYRO_FULLSCALE_DPS,
        .resolution = GYRO_FULLSCALE_DPS / GYRO_QUANTIZATION_LEVELS,
        .power = GYRO_MA,
        .minDelay = GYRO_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_GYROSCOPE_UNCALIBRATED,
        .requiredPermission = "",
        .maxDelay = GYRO_MAX_DELAY_US,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} },
    { .name = "Game Rotation Vector",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_GAME_RV,
        .type = SENSOR_TYPE_GAME_ROTATION_VECTOR,
        .maxRange = 1.0f,
        .resolution = 1.0f / RV_QUANTIZATION_LEVELS,
        .power = ACCEL_MA + GYRO_MA + MOT_GAMERV_MA,
        .minDelay = GYRO_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_GAME_ROTATION_VECTOR,
        .requiredPermission = "",
        .maxDelay = FUSION_MAX_DELAY_US,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} },
    { .name = "Gravity",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_GRAVITY,
        .type = SENSOR_TYPE_GRAVITY,
        .maxRange = GRAVITY_EARTH,
        .resolution = GRAVITY_EARTH / GRAV_QUANTIZATION_LEVELS,
        .power = ACCEL_MA + GYRO_MA + MOT_LAGRAV_MA,
        .minDelay = ACCEL_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_GRAVITY,
        .requiredPermission = "",
        .maxDelay = FUSION_MAX_DELAY_US,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} },
    { .name = "Linear Acceleration",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE+ID_LA,
        .type = SENSOR_TYPE_LINEAR_ACCELERATION,
        .maxRange = ACCEL_FULLSCALE_G * GRAVITY_EARTH,
        .resolution = GRAVITY_EARTH / LSG,
        .power = ACCEL_MA + GYRO_MA + MOT_LAGRAV_MA,
        .minDelay = ACCEL_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_LINEAR_ACCELERATION,
        .requiredPermission = "",
        .maxDelay = FUSION_MAX_DELAY_US,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} },
#endif /* _ENABLE_GYROSCOPE */
#ifdef _ENABLE_PEDO
    { .name = "Step Detector sensor",
        .vendor = VENDOR_ACCEL,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE+ID_STEP_DETECTOR,
        .type = SENSOR_TYPE_STEP_DETECTOR,
        .maxRange = 1.0f,
        .resolution = 0,
        .power = 0,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = "",
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_SPECIAL_REPORTING_MODE,
        .reserved = {0,0} },
    { .name = "Step Counter sensor",
        .vendor = VENDOR_ACCEL,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE+ID_STEP_COUNTER,
        .type = SENSOR_TYPE_STEP_COUNTER,
        .maxRange = FLT_MAX,
        .resolution = 0,
        .power = 0,
        .minDelay = STEP_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = "",
        .requiredPermission = "",
        .maxDelay = STEP_MAX_DELAY_US,
        .flags = SENSOR_FLAG_ON_CHANGE_MODE,
        .reserved = {0,0} },
#endif
    { .name = "Ambient Light sensor",
        .vendor = VENDOR_PROXALS,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_L,
        .type = SENSOR_TYPE_LIGHT,
        .maxRange = ALS_FULLSCALE_LUX,
        .resolution = ALS_FULLSCALE_LUX / ALS_QUANTIZATION_LEVELS,
        .power = ALS_MA,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_LIGHT,
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_ON_CHANGE_MODE,
        .reserved = {0,0} },
    { .name = "Proximity sensor",
        .vendor = VENDOR_PROXALS,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_P,
        .type = SENSOR_TYPE_PROXIMITY,
        .maxRange = 100.0f,
        .resolution = 100.0f,
        .power = PROX_MA,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_PROXIMITY,
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
        .reserved = {0,0} },
    { .name = "Display Rotation sensor",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_DR,
        .type = SENSOR_TYPE_DISPLAY_ROTATE,
        .maxRange = 4.0f,
        .resolution = 1.0f,
        .power = ACCEL_MA,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_DISPLAY_ROTATE,
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_ON_CHANGE_MODE,
        .reserved = {0,0} },
    { .name = "Flat Up",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_FU,
        .type = SENSOR_TYPE_FLAT_UP,
        .maxRange = 1.0f,
        .resolution = 1.0f,
        .power = ACCEL_MA,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = "com.motorola.sensor.flat_up",
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
        .reserved = {0,0} },
    { .name = "Flat Down",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_FD,
        .type = SENSOR_TYPE_FLAT_DOWN,
        .maxRange = 1.0f,
        .resolution = 1.0f,
        .power = ACCEL_MA,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = "com.motorola.sensor.flat_down",
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
        .reserved = {0,0} },
    { .name = "Stowed",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_S,
        .type = SENSOR_TYPE_STOWED,
        .maxRange = 1.0f,
        .resolution = 1.0f,
        .power = PROX_MA,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = "com.motorola.sensor.stowed",
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
        .reserved = {0,0} },
    { .name = "Camera Activation sensor",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_CA,
        .type = SENSOR_TYPE_CAMERA_ACTIVATE,
        .maxRange = 1.0f,
        .resolution = 1.0f,
        .power = CAM_ACT_ALGO_MA,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = "com.motorola.sensor.camera_activate",
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_SPECIAL_REPORTING_MODE | SENSOR_FLAG_WAKE_UP,
        .reserved = {0,0} },
#ifdef _ENABLE_CHOPCHOP
    { .name = "ChopChop Gesture",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_CC,
        .type = SENSOR_TYPE_CHOPCHOP_GESTURE,
        .maxRange = 1.0f,
        .resolution = 1.0f,
        .power = CHOP_CHOP_ALGO_MA,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = "com.motorola.sensor.chopchop",
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_SPECIAL_REPORTING_MODE | SENSOR_FLAG_WAKE_UP,
        .reserved = {0,0} },
#endif
#ifdef _ENABLE_LIFT
    { .name = "Lift Gesture Virtual Sensor",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_LF,
        .type = SENSOR_TYPE_LIFT_GESTURE,
        .maxRange = 1.0f,
        .resolution = 1.0f,
        .power = LIFT_ALGO_MA,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = "com.motorola.sensor.lift",
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_SPECIAL_REPORTING_MODE | SENSOR_FLAG_WAKE_UP,
        .reserved = {0,0} },
#endif
#ifdef _ENABLE_ACCEL_SECONDARY
    { .name = "3-axis Accelerometer, Secondary",
        .vendor = VENDOR_ACCEL,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_A2,
        .type = SENSOR_TYPE_ACCELEROMETER,
        .maxRange = ACCEL_FULLSCALE_G * GRAVITY_EARTH,
        .resolution = GRAVITY_EARTH / LSG,
        .power = ACCEL_MA,
        .minDelay = ACCEL_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_ACCELEROMETER,
        .requiredPermission = "",
        .maxDelay = ACCEL_MAX_DELAY_US,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} },
#endif
#ifdef _ENABLE_REARPROX
    { .name = "Rear Proximity sensor",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_RP,
        .type = SENSOR_TYPE_PROXIMITY,
        .maxRange = 100.0f,
        .resolution = 100.0f,
        .power = 0.35f,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_PROXIMITY,
        .requiredPermission = "",
        .maxDelay = 10000000,
        .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
        .reserved = { 0, 0 } },
#endif
#ifdef _ENABLE_REARPROX_2
 { .name = "Rear Proximity sensor, Secondary",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_RP_2,
        .type = SENSOR_TYPE_PROXIMITY,
        .maxRange = 100.0f,
        .resolution = 100.0f,
        .power = 0.35f,
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_PROXIMITY,
        .requiredPermission = "",
        .maxDelay = 10000000,
        .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
        .reserved = { 0, 0 } },
#endif
/*    { .name = "Glance Gesture",
      .vendor = VENDOR_MOT,
      .version = 1,
      .handle = SENSORS_HANDLE_BASE+ID_GLANCE_GESTURE,
      .type = SENSOR_TYPE_GLANCE_GESTURE,
      .maxRange = 1.0f,
      .resolution = 1.0f,
      .power = GLANCE_ALGO_MA,
      .minDelay = 0,
      .fifoReservedEventCount = 0,
      .fifoMaxEventCount = 0,
      .stringType = SENSOR_STRING_TYPE_GLANCE_GESTURE,
      .requiredPermission = "",
      .maxDelay = 0,
      .flags = SENSOR_FLAG_SPECIAL_REPORTING_MODE | SENSOR_FLAG_WAKE_UP,
      .reserved = {0,0}},*/
    { .name = "Moto Glance Gesture",
      .vendor = VENDOR_MOT,
      .version = 1,
      .handle = SENSORS_HANDLE_BASE+ID_MOTO_GLANCE_GESTURE,
      .type = SENSOR_TYPE_MOTO_GLANCE_GESTURE,
      .maxRange = 1.0f,
      .resolution = 1.0f,
      .power = GLANCE_ALGO_MA,
      .minDelay = 0,
      .fifoReservedEventCount = 0,
      .fifoMaxEventCount = 0,
      .stringType = SENSOR_STRING_TYPE_MOTO_GLANCE_GESTURE,
      .requiredPermission = "",
      .maxDelay = 0,
      .flags = SENSOR_FLAG_SPECIAL_REPORTING_MODE | SENSOR_FLAG_WAKE_UP,
      .reserved = {0,0}},
    { .name = "Motion Detect",
      .vendor = VENDOR_MOT,
      .version = 1,
      .handle = SENSORS_HANDLE_BASE + ID_MOTION_DETECT,
      .type = SENSOR_TYPE_MOTION_DETECT,
      .maxRange = 1.0f,
      .resolution = 1.0f,
      .power = ACCEL_MA,
      .minDelay = -1,
      .fifoReservedEventCount = 0,
      .fifoMaxEventCount = 0,
      .stringType = SENSOR_STRING_TYPE_MOTION_DETECT,
      .requiredPermission = "",
      .maxDelay = 0,
      .flags = SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP,
      .reserved = {0,0}},
    { .name = "Stationary Detect",
      .vendor = VENDOR_MOT,
      .version = 1,
      .handle = SENSORS_HANDLE_BASE + ID_STATIONARY_DETECT,
      .type = SENSOR_TYPE_STATIONARY_DETECT,
      .maxRange = 1.0f,
      .resolution = 1.0f,
      .power = ACCEL_MA,
      .minDelay = -1,
      .fifoReservedEventCount = 0,
      .fifoMaxEventCount = 0,
      .stringType = SENSOR_STRING_TYPE_STATIONARY_DETECT,
      .requiredPermission = "",
      .maxDelay = 0,
      .flags = SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP,
      .reserved = {0,0}},
};

#ifdef _ENABLE_MAGNETOMETER
const struct sensor_t threeAxCalMagSensorType = {
	.name = "3-axis Calibrated Magnetic field sensor",
        .vendor = VENDOR_MAG,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_M,
        .type = SENSOR_TYPE_MAGNETIC_FIELD,
        .maxRange = MAG_FULLSCALE_UT,
        .resolution = CONVERT_M,
        .power = MAG_MA,
        .minDelay = MAG_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_MAGNETIC_FIELD,
        .requiredPermission = "",
        .maxDelay = MAG_MAX_DELAY_US,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} };

const struct sensor_t threeAxunCalMagSensorType = {
	.name = "3-axis Uncalibrated Magnetic field sensor",
        .vendor = VENDOR_MAG,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_UM,
        .type = SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED,
        .maxRange = MAG_FULLSCALE_UT,
        .resolution = CONVERT_M,
        .power = MAG_MA,
        .minDelay = MAG_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_MAGNETIC_FIELD_UNCALIBRATED,
        .requiredPermission = "",
        .maxDelay = MAG_MAX_DELAY_US,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} };

const struct sensor_t orientationSensorType = {
	.name = "Orientation sensor",
        .vendor = VENDOR_MAG,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_OR,
        .type = SENSOR_TYPE_ORIENTATION,
        .maxRange = 360.0f,
        .resolution = CONVERT_OR,
        .power = MAG_MA + ACCEL_MA + ORIENT_ALGO_MA,
        .minDelay = MAG_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_ORIENTATION,
        .requiredPermission = "",
        .maxDelay = MAG_MAX_DELAY_US,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} };

const struct sensor_t geoRotationSensorType = {
	.name = "Geomagnetic Rotation Vector",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_GEOMAG_RV,
        .type = SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR,
        .maxRange = 1.0f,
        .resolution = 1.0f / RV_QUANTIZATION_LEVELS,
        .power = ACCEL_MA + MAG_MA,
        .minDelay = ACCEL_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_GEOMAGNETIC_ROTATION_VECTOR,
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} };

const struct sensor_t rotationSensorType = {
	.name = "Rotation Vector",
        .vendor = VENDOR_MOT,
        .version = 1,
        .handle = SENSORS_HANDLE_BASE + ID_RV,
        .type = SENSOR_TYPE_ROTATION_VECTOR,
        .maxRange = 1.0f,
        .resolution = 1.0f / RV_QUANTIZATION_LEVELS,
        .power = ACCEL_MA + GYRO_MA + MAG_MA,
        .minDelay = GYRO_MIN_DELAY_US,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = SENSOR_STRING_TYPE_ROTATION_VECTOR,
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved = {0,0} };
#endif

#ifdef _ENABLE_CAPSENSE
const struct sensor_t capSensorType ={ 
	.name = "Moto CapSense",
	.vendor = VENDOR_MOT,
	.version = 1,
	.handle = SENSORS_HANDLE_BASE + ID_CS,
	.type = SENSOR_TYPE_MOTO_CAPSENSE,
	.maxRange = 100.0f,
	.resolution = 100.0f,
	.power = 0.35f,
	.minDelay = 0,
	.fifoReservedEventCount = 0,
	.fifoMaxEventCount = 0,
	.stringType = SENSOR_STRING_TYPE_MOTO_CAPSENSE,
	.requiredPermission = "",
	.maxDelay = 10000000,
	.flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
	.reserved = { 0, 0 } };
#endif

/* Clean up definitions */
#undef VENDOR_ACCEL
#undef VENDOR_GYRO
#undef VENDOR_MAG
#undef VENDOR_MOT
#undef VENDOR_PROXALS

#undef ACCEL_MA
#undef GYRO_MA
#undef MAG_MA
#undef ALS_MA
#undef PROX_MA

#undef ORIENT_ALGO_MA
#undef CAM_ACT_ALGO_MA
#undef CHOP_CHOP_ALGO_MA
#undef LIFT_ALGO_MA
#undef MOT_GAMERV_MA
#undef MOT_LAGRAV_MA

#undef ACCEL_FULLSCALE_G
#undef GYRO_FULLSCALE_DPS
#undef MAG_FULLSCALE_UT
#undef ALS_FULLSCALE_LUX

#undef GYRO_QUANTIZATION_LEVELS
#undef ALS_QUANTIZATION_LEVELS
#undef RV_QUANTIZATION_LEVELS
#undef GRAV_QUANTIZATION_LEVELS

/*
 * Copyright (C) 2009-2015 Motorola Mobility
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
#include "mot_sensorhub_motosh.h"

#include "Sensors.h"
#include "HubSensors.h"

/*****************************************************************************/

/*
 * The SENSORS Module
 */

/* Combo part conditional compilation choices */
#ifdef _USES_ICM20645_ACCGYR
#define VENDOR_ACCEL  "InvenSense"
#define VENDOR_GYRO   "InvenSense"
#define ACCEL_PART_NO "ICM20645"
#define GYRO_PART_NO  "ICM20645"
/* Current drain figures from spec sheets in mA */
#define ACCGYR_6AXIS_MA          3.4f
#define ACCGYR_GYRO_MA           3.2f
#define ACCGYR_ACCEL_MA          0.45f
#define ACCGYR_ACCEL_LOWPOWER_MA 0.019f
#endif

#ifdef _USES_BMI160_ACCGYR
#define VENDOR_ACCEL  "Bosch"
#define VENDOR_GYRO   "Bosch"
#define ACCEL_PART_NO "BMI160"
#define GYRO_PART_NO  "BMI160"
/* Current drain figures from spec sheets in mA */
#define ACCGYR_6AXIS_MA          0.95f
#define ACCGYR_GYRO_MA           0.90f
#define ACCGYR_ACCEL_MA          0.55f
#define ACCGYR_ACCEL_LOWPOWER_MA 0.02f
#endif

/* Vendor names */
#define VENDOR_MAG  "Asahi Kasei"
#define VENDOR_MOT  "Motorola"
#define VENDOR_PROXALS "TAOS"

/* Range settings */
#define ACCEL_FULLSCALE_G  (16.f)
#define GYRO_FULLSCALE_DPS (2000.f)
#define MAG_FULLSCALE_UT   (4900.f)
#define ALS_FULLSCALE_LUX  ((float)UINT16_MAX)

/* Resolution settings */
#define GYRO_QUANTIZATION_LEVELS  (INT16_MAX)
#define MAG_QUANTIZATION_LEVELS   (INT16_MAX)
#define ALS_QUANTIZATION_LEVELS   (UINT16_MAX)
#define RV_QUANTIZATION_LEVELS    (INT16_MAX)
#define GRAV_QUANTIZATION_LEVELS  (INT16_MAX)

/* Min delays */
#define ACCEL_MIN_DELAY_US     5000
#define GYRO_MIN_DELAY_US      5000
#define MAG_MIN_DELAY_US       20000
#define STEP_MIN_DELAY_US      1000000 /* 1 sec */
#define MOTO_MOD_MIN_DELAY_US  1000000 /* 1 sec */

/* Max delays */
#define ACCEL_MAX_DELAY_US     200000
#define GYRO_MAX_DELAY_US      200000
#define MAG_MAX_DELAY_US       200000
#define STEP_MAX_DELAY_US      0
#define MOTO_MOD_MAX_DELAY_US  0

/* Part numbers to use in sensor names */
#define MAG_PART_NO   "AKM09912"
#define ALS_PART_NO   "CT1011"
#define PROX_PART_NO  "CT1011"

/* Various current drain figures from spec sheets in mA */
#define MAG_MA              1.0f
#define ALS_MA              0.25f
#define PROX_MA             0.0467f /* 100mA (LED drive) * (7us/pulse * 4 pulses)/60ms */

/* Estimated algorithm current draw in mA*/
#define ORIENT_ALGO_MA  1.0f
#define CAM_ACT_ALGO_MA 1.0f
#define MOT_9AXIS_MA    1.0f
#define MOT_6AXIS_MA    1.0f
#define MOT_LAGRAV_MA   1.0f
#define GLANCE_ALGO_MA    (ACCGYR_ACCEL_MA + PROX_MA + ALS_MA)

const std::vector<struct sensor_t> & HubSensors::hubSensorList() {

    static const std::vector<struct sensor_t> sSensorList = {
    {
                .name = ACCEL_PART_NO " 3-axis Accelerometer",
                .vendor = VENDOR_ACCEL,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_A,
                .type = SENSOR_TYPE_ACCELEROMETER,
                .maxRange = ACCEL_FULLSCALE_G*GRAVITY_EARTH,
                .resolution = GRAVITY_EARTH/LSG,
                .power = ACCGYR_ACCEL_MA,
                .minDelay = ACCEL_MIN_DELAY_US,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 500,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = ACCEL_MAX_DELAY_US,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },
    {
                .name = GYRO_PART_NO " Gyroscope",
                .vendor = VENDOR_GYRO,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_G,
                .type = SENSOR_TYPE_GYROSCOPE,
                .maxRange = GYRO_FULLSCALE_DPS,
                .resolution = GYRO_FULLSCALE_DPS / GYRO_QUANTIZATION_LEVELS,
                .power = ACCGYR_GYRO_MA,
                .minDelay = GYRO_MIN_DELAY_US,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = GYRO_MAX_DELAY_US,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },
    {
                .name = MAG_PART_NO " 3-axis Magnetometer",
                .vendor = VENDOR_MAG,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_M,
                .type = SENSOR_TYPE_MAGNETIC_FIELD,
                .maxRange = MAG_FULLSCALE_UT,
                .resolution = MAG_FULLSCALE_UT / MAG_QUANTIZATION_LEVELS,
                .power = MAG_MA,
                .minDelay = MAG_MIN_DELAY_US,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = MAG_MAX_DELAY_US,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },
    {
                .name = MAG_PART_NO " Orientation",
                .vendor = VENDOR_MAG,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_O,
                .type = SENSOR_TYPE_ORIENTATION,
                .maxRange = 360.0f,
                .resolution = 1.0f/64.0f,
                .power = MAG_MA + ACCGYR_ACCEL_MA + ORIENT_ALGO_MA,
                .minDelay = MAG_MIN_DELAY_US,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = MAG_MAX_DELAY_US,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },
    {
                .name = ALS_PART_NO " Ambient Light",
                .vendor = VENDOR_PROXALS,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_L,
                .type = SENSOR_TYPE_LIGHT,
                .maxRange = ALS_FULLSCALE_LUX,
                .resolution = ALS_FULLSCALE_LUX / ALS_QUANTIZATION_LEVELS,
                .power = ALS_MA,
                .minDelay = 0,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ON_CHANGE_MODE,
                .reserved = {0,0} },
    {
                .name = "Display Rotation",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_DR,
                .type = SENSOR_TYPE_DISPLAY_ROTATE,
                .maxRange = 4.0f,
                .resolution = 1.0f,
                .power = ACCGYR_ACCEL_MA,
                .minDelay = 0,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = SENSOR_STRING_TYPE_DISPLAY_ROTATE,
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ON_CHANGE_MODE,
                .reserved = {0,0} },
    {
                .name = PROX_PART_NO " Proximity",
                .vendor = VENDOR_PROXALS,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_P,
                .type = SENSOR_TYPE_PROXIMITY,
                .maxRange = 100.0f,
                .resolution = 100.0f,
                .power = PROX_MA,
                .minDelay = 0,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0}},
    {
                .name = "Flat Up",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_FU,
                .type = SENSOR_TYPE_FLAT_UP,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = ACCGYR_ACCEL_MA,
                .minDelay = 0,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "com.motorola.sensor.flat_up",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0}},
    {
                .name = "Flat Down",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_FD,
                .type = SENSOR_TYPE_FLAT_DOWN,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = ACCGYR_ACCEL_MA,
                .minDelay = 0,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "com.motorola.sensor.flat_down",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0}},
    {
                .name = "Stowed",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_S,
                .type = SENSOR_TYPE_STOWED,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = ALS_MA + PROX_MA,
                .minDelay = 0,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "com.motorola.sensor.stowed",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0}},
    {
                .name = "Camera Activation",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_CA,
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
                .reserved = {0,0}},
#ifdef _ENABLE_IR
    {
                .name = "IR Gestures",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_IR_GESTURE,
                .type = SENSOR_TYPE_IR_GESTURE,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = 1.0f,
                .minDelay = 0,
                .fifoReservedEventCount = 8,
                .fifoMaxEventCount = 8,
                .stringType = "com.motorola.sensor.ir_gesture",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0}},
#ifdef _ENABLE_RAW_IR_DATA
    {
                .name = "IR Raw Data",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_IR_RAW,
                .type = SENSOR_TYPE_IR_RAW,
                .maxRange = 4096.0f,
                .resolution = 1.0f,
                .power = 1.0f,
                .minDelay = 10000,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "com.motorola.sensor.ir_raw",
                .requiredPermission = "",
                .maxDelay = 200000,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },
#endif /* _ENABLE_RAW_IR_DATA */
    {
                .name = "IR Object Detect",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_IR_OBJECT,
                .type = SENSOR_TYPE_IR_OBJECT,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = 1.0f,
                .minDelay = -1,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "com.motorola.sensor.ir_object",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ONE_SHOT_MODE,
                .reserved = {0,0} },
#endif /* _ENABLE_IR */
    {
                .name = "Significant Motion",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_SIM,
                .type = SENSOR_TYPE_SIGNIFICANT_MOTION,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = 3.0f,
                .minDelay = -1,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0}},
#ifdef _ENABLE_PEDO
    {
                .name = "Step Detector sensor",
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

    {
                .name = "Step Counter sensor",
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
    {
                .name = GYRO_PART_NO " Uncalibrated Gyroscope",
                .vendor = VENDOR_GYRO,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_UNCALIB_GYRO,
                .type = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED,
                .maxRange = GYRO_FULLSCALE_DPS,
                .resolution = GYRO_FULLSCALE_DPS / GYRO_QUANTIZATION_LEVELS,
                .power = ACCGYR_GYRO_MA,
                .minDelay = GYRO_MIN_DELAY_US,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = GYRO_MAX_DELAY_US,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },
    {
                .name = MAG_PART_NO " 3-axis Uncalibrated Magnetometer",
                .vendor = VENDOR_MAG,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_UNCALIB_MAG,
                .type = SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED,
                .maxRange = MAG_FULLSCALE_UT,
                .resolution = MAG_FULLSCALE_UT / MAG_QUANTIZATION_LEVELS,
                .power = MAG_MA,
                .minDelay = MAG_MIN_DELAY_US,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = MAG_MAX_DELAY_US,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },
#ifdef _ENABLE_CHOPCHOP
    {
                .name = "ChopChop Gesture",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_CHOPCHOP_GESTURE,
                .type = SENSOR_TYPE_CHOPCHOP_GESTURE,
                .maxRange = USHRT_MAX*1.0f,
                .resolution = 1.0f,
                .power = 0.0f,
                .minDelay = 0,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_SPECIAL_REPORTING_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0} },
#endif
#ifdef _ENABLE_LIFT
    {
                .name = "Lift Gesture",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_LIFT_GESTURE,
                .type = SENSOR_TYPE_LIFT_GESTURE,
                .maxRange = USHRT_MAX*1.0f,
                .resolution = 1.0f,
                .power = 0.0f,
                .minDelay = 0,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_SPECIAL_REPORTING_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0} },
#endif
    {
                .name = "Rotation Vector",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_QUAT_9AXIS,
                .type = SENSOR_TYPE_ROTATION_VECTOR,
                .maxRange = 1.0f,
                .resolution = 1.0f / RV_QUANTIZATION_LEVELS,
                .power = ACCGYR_6AXIS_MA + MAG_MA + MOT_9AXIS_MA,
                .minDelay = ACCEL_MIN_DELAY_US,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },
    {
                .name = "Geomagnetic Rotation Vector",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_QUAT_6AXIS,
                .type = SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR,
                .maxRange = 1.0f,
                .resolution = 1.0f / RV_QUANTIZATION_LEVELS,
                .power = ACCGYR_6AXIS_MA + MOT_6AXIS_MA,
                .minDelay = ACCEL_MIN_DELAY_US,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },

    {
                .name = "Game Rotation Vector",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_GAME_RV,
                .type = SENSOR_TYPE_GAME_ROTATION_VECTOR,
                .maxRange = 1.0f,
                .resolution = 1.0f / RV_QUANTIZATION_LEVELS,
                .power = ACCGYR_6AXIS_MA,
                .minDelay = ACCEL_MIN_DELAY_US,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },
#ifdef _ENABLE_GR
    {
                .name = "Gravity",
                .vendor = "Motorola",
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_GRAVITY,
                .type = SENSOR_TYPE_GRAVITY,
                .maxRange = GRAVITY_EARTH,
                .resolution = GRAVITY_EARTH / GRAV_QUANTIZATION_LEVELS,
                .power = ACCGYR_ACCEL_MA + ACCGYR_GYRO_MA + MAG_MA + MOT_LAGRAV_MA,
                .minDelay = 10000,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },
#endif
#ifdef _ENABLE_LA
    {
                .name = "Linear Acceleration",
                .vendor = "Motorola",
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_LA,
                .type = SENSOR_TYPE_LINEAR_ACCELERATION,
                .maxRange = ACCEL_FULLSCALE_G*GRAVITY_EARTH,
                .resolution = GRAVITY_EARTH/LSG,
                .power = ACCGYR_ACCEL_MA + ACCGYR_GYRO_MA + MAG_MA + MOT_LAGRAV_MA,
                .minDelay = 10000,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = "",
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_CONTINUOUS_MODE,
                .reserved = {0,0} },
#endif
    {
                .name = "Glance Gesture",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_GLANCE_GESTURE,
                .type = SENSOR_TYPE_GLANCE_GESTURE,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = GLANCE_ALGO_MA,
                .minDelay = -1,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = SENSOR_STRING_TYPE_GLANCE_GESTURE,
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0}},
    {
                .name = "Moto Glance Gesture",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_MOTO_GLANCE_GESTURE,
                .type = SENSOR_TYPE_MOTO_GLANCE_GESTURE,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = GLANCE_ALGO_MA,
                .minDelay = -1,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = SENSOR_STRING_TYPE_MOTO_GLANCE_GESTURE,
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0}},
    {
                .name = "Moto Mod Current Drain",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_MOTO_MOD_CURRENT_DRAIN,
                .type = SENSOR_TYPE_MOTO_MOD_CURRENT_DRAIN,
                .maxRange = FLT_MAX,
                .resolution = 1.0f,
                .power = 0.0f,
                .minDelay = MOTO_MOD_MIN_DELAY_US,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = SENSOR_STRING_TYPE_MOTO_MOD_CURRENT_DRAIN,
                .requiredPermission = "",
                .maxDelay = MOTO_MOD_MAX_DELAY_US,
                .flags = SENSOR_FLAG_ON_CHANGE_MODE,
                .reserved = {0,0} },
#ifdef _CAMFSYNC
    {
                .name = "Sensor Sync",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_SENSOR_SYNC,
                .type = SENSOR_TYPE_SENSOR_SYNC,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = 0.0f,
                .minDelay = 0,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = SENSOR_STRING_TYPE_SENSOR_SYNC,
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_SPECIAL_REPORTING_MODE,
                .reserved = {0,0}},
#endif /* _CAMFSYNC */

#ifdef _ENABLE_ULTRASOUND
    {
                .name = "Ultrasound Gestures",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE+ID_ULTRASOUND_GESTURE,
                .type = SENSOR_TYPE_ULTRASOUND_GESTURE,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = 0.0f,
                .minDelay = 0,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = SENSOR_STRING_TYPE_ULTRASOUND_GESTURE,
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_SPECIAL_REPORTING_MODE,
                .reserved = {0,0}},
#endif /* _ENABLE_ULTRASOUND */

    {
                .name = "Motion Detect",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE + ID_MOTION_DETECT,
                .type = SENSOR_TYPE_MOTION_DETECT,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = ACCGYR_ACCEL_MA,
                .minDelay = -1,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = SENSOR_STRING_TYPE_MOTION_DETECT,
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0}},
    {
                .name = "Stationary Detect",
                .vendor = VENDOR_MOT,
                .version = 1,
                .handle = SENSORS_HANDLE_BASE + ID_STATIONARY_DETECT,
                .type = SENSOR_TYPE_STATIONARY_DETECT,
                .maxRange = 1.0f,
                .resolution = 1.0f,
                .power = ACCGYR_ACCEL_MA,
                .minDelay = -1,
                .fifoReservedEventCount = 0,
                .fifoMaxEventCount = 0,
                .stringType = SENSOR_STRING_TYPE_STATIONARY_DETECT,
                .requiredPermission = "",
                .maxDelay = 0,
                .flags = SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP,
                .reserved = {0,0}},
    };

    return sSensorList;
}

/* Clean up definitions */
#undef VENDOR_MAG
#undef VENDOR_ACCEL
#undef VENDOR_GYRO
#undef VENDOR_MOT
#undef VENDOR_PROXALS

#undef ACCEL_MIN_DELAY_US
#undef GYRO_MIN_DELAY_US
#undef MAG_MIN_DELAY_US

#undef ACCEL_MAX_DELAY_US
#undef GYRO_MAX_DELAY_US
#undef MAG_MAX_DELAY_US

#undef ACCEL_PART_NO
#undef GYRO_PART_NO
#undef MAG_PART_NO
#undef ALS_PART_NO

#undef ACCGYR_6AXIS_MA
#undef ACCGYR_GYRO_MA
#undef ACCGYR_ACCEL_MA
#undef ACCGYR_ACCEL_LOWPOWER_MA
#undef MAG_MA
#undef ALS_MA
#undef PROX_MA

#undef ORIENT_ALGO_MA
#undef CAM_ACT_ALGO_MA
#undef MOT_9AXIS_MA

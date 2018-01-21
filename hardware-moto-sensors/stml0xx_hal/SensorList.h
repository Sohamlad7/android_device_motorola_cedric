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

#ifndef SENSORLIST_H
#define SENSORLIST_H

#include <float.h>
#include <limits.h>
#include <vector>

#include <hardware/sensors.h>
#include "mot_sensorhub_stml0xx.h"

#include "Sensors.h"

#define ACCEL_MAX_DELAY_US  200000
#define ACCEL_MIN_DELAY_US  10000

#define GYRO_MAX_DELAY_US   200000
#define GYRO_MIN_DELAY_US   5000

#define MAG_MAX_DELAY_US    200000
#define MAG_MIN_DELAY_US    20000

#define FUSION_MAX_DELAY_US 10000

extern std::vector<struct sensor_t> sSensorList;
#ifdef _ENABLE_MAGNETOMETER
extern const struct sensor_t threeAxCalMagSensorType;
extern const struct sensor_t threeAxunCalMagSensorType;
extern const struct sensor_t orientationSensorType;
extern const struct sensor_t geoRotationSensorType;
extern const struct sensor_t rotationSensorType;
#endif // _ENABLE_MAGNETOMETER
#ifdef _ENABLE_CAPSENSE
extern const struct sensor_t capSensorType;
#endif // _ENABLE_CAPSENSE
#endif // SENSORLIST_H


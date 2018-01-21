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
 * Copyright (C) 2016 Motorola Mobility LLC
 */

#include <float.h>
#include <math.h>
#include <string.h>
#include <cutils/log.h>
#include "FusionSensorBase.h"
#include "GyroIntegration.h"
#include "Quaternion.h"
#include "SensorList.h"

void GyroIntegration::integrate(
    struct GyroIntegrationState* gis,
    QuatData& rvOut,
    const QuatData& rvIn,
    FusionData& fusionData
)
{
    // Time (s) from the last call to this function.
    float dt;
    // Last gyro sample (rad/s)
    float gyroX;
    float gyroY;
    float gyroZ;
    // Magnitude of gyro vector above.
    float gyroMag;
    // \theta/2, where \theta is the rotation angle implied by the gyro
    // vector above.
    float halfTheta;
    // Trig
    float sinHalfTheta;
    float cosHalfTheta;
    // Incremental rotation quaternion implied by (gyroX,gyroY,gyroZ).
    QuatData quatGyroDelta;
    float nine_axis_filter;

    // If it becomes necessary to reset the RV, provide a call
    // to set gis->initialized=0 and done.
    if ( !gis->initialized ) {
        gis->quatGyro.a = rvIn.a;
        gis->quatGyro.b = rvIn.b;
        gis->quatGyro.c = rvIn.c;
        gis->quatGyro.d = rvIn.d;
        gis->quatGyro.timestamp = rvIn.timestamp;
        gis->initialized = 1;
    }

    // 1) Forward-integrate the gyro
    if (fusionData.gyro.timestamp != gis->quatGyro.timestamp) {
        dt = (float)(fusionData.gyro.timestamp - gis->quatGyro.timestamp) / 1000000000.f;
        gis->quatGyro.timestamp = fusionData.gyro.timestamp;
    } else {
        dt = (float)GYRO_MIN_DELAY_US / 1000000.f;
    }

    gyroX = fusionData.gyro.x;
    gyroY = fusionData.gyro.y;
    gyroZ = fusionData.gyro.z;
    // Normalize the gyro vector if we can do so with single precision.
    gyroMag = gyroX*gyroX + gyroY*gyroY + gyroZ*gyroZ;
    // NOTE: the tolerance should be at least approximately machineEps^2,
    //       since we will take a sqrt.
    if (gyroMag > (float)1e-5) {
        gyroMag = sqrtf(gyroMag);
        gyroX /= gyroMag;
        gyroY /= gyroMag;
        gyroZ /= gyroMag;
        halfTheta = dt * gyroMag / 2.f;
    } else {
        // If it is too small to be normalized, let's just say there is
        // no rotation.
        halfTheta = 0.f;
    }

    // Construct incremental gyro rotation quaternion
    sinHalfTheta = sinf(halfTheta);
    cosHalfTheta = cosf(halfTheta);
    quatGyroDelta.a = sinHalfTheta * gyroX;
    quatGyroDelta.b = sinHalfTheta * gyroY;
    quatGyroDelta.c = sinHalfTheta * gyroZ;
    quatGyroDelta.d = cosHalfTheta;
    Quaternion::renormalize(quatGyroDelta);

    // Multiply to apply incremental rotation (integration). The order is
    // first to rotate by quatGyro, then apply incremental rotation
    // quatGyroDelta. So, we update quatGyro with the argument
    // quatGyroDelta on the right, and quatGyro on the left.
    //   newQuatGyro = quatGyro * quatGyroDelta
    if (Quaternion::mul(gis->quatGyro, gis->quatGyro, quatGyroDelta)) {
        ALOGD("gyroIntegration: quatMul bad");
        gis->initialized = 0;
        return;
    }

    // Select an alpha depending on how fast we are rotating
    if (gyroMag < NINE_AXIS_FILTER_SELECT_THRESH)
        nine_axis_filter = NINE_AXIS_FILTER_ALPHA_SLOW;
    else
        nine_axis_filter = NINE_AXIS_FILTER_ALPHA_FAST;

    // 3) Fuse gyro quaternion to six-axis quaternion, and feedback result
    //    to the gyro quaternion. Voila!
    Quaternion::linInterp(gis->quatGyro, rvIn, gis->quatGyro, nine_axis_filter);

    // Copy out
    rvOut = gis->quatGyro;
}

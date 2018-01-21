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

#include <float.h>
#include <math.h>
#include <string.h>
#include <cutils/log.h>
#include <android-base/macros.h>
#include "LinearAccelGravity.h"
#include "Quaternion.h"
#include "SensorList.h"

LinearAccelGravity LinearAccelGravity::self;

LinearAccelGravity::LinearAccelGravity()
: gameRVData(), gameRVts(0)
{
}

LinearAccelGravity::~LinearAccelGravity()
{
}

LinearAccelGravity *LinearAccelGravity::getInstance()
{
    return &self;
}

bool LinearAccelGravity::processFusion(FusionData& fusionData, bool reset)
{
    UNUSED(reset);

    /*
     * In the android-defined coordinate system (x east, y north, z up),
     * gravity is along the z axis.
     *
     * So, we need to convert the z vector (0,0,1) to a quaternion, then
     * rotate by the 9-axis vector to get the current gravity. To do this,
     * first embed the z vector in a quaternion:
     *
     *     qz := 0*i + 0*j + 1*k + 0.
     *
     * Then, supposing q is the current 9-axis rv:
     *
     *     qGrav = q^(-1)*qz*q,
     *
     * where q^(-1) is the inverse of q. Then, we extract the resulting vector
     * (gx, gy, gz) from qGrav:
     *
     *     qGrav = gx*i + gy*j + gz*k + 0.
     */

    QuatData qz = {0.f, 0.f, 1.f, 0.f, 0.f, 0};
    QuatData qGrav = {0.f, 0.f, 0.f, 1.f, 0.f, 0};
    float mag = 0.f;

    /* qGrav = gameRV^{-1} * qz * gameRV */
    Quaternion::inv(qGrav, fusionData.gameRotation);
    Quaternion::mul(qz, qz, fusionData.gameRotation);
    Quaternion::mul_noRenormalize(qGrav, qGrav, qz);

    /* We explicitly requested the last quatMul not to renormalize. The reason
     * is that theoretically qGrav[3] == 0, but due to numerical issues, it will be
     * some very small positive or NEGATIVE number, which can cause the gravity
     * vector to flip signs. Handle the normalization explicitly here without
     * sign changes.
     */
    mag = sqrtf(qGrav.a * qGrav.a + qGrav.b * qGrav.b + qGrav.c * qGrav.c + qGrav.d * qGrav.d);
    fusionData.gravity.x = (qGrav.a / mag) * GRAVITY_EARTH;
    fusionData.gravity.y = (qGrav.b / mag) * GRAVITY_EARTH;
    fusionData.gravity.z = (qGrav.c / mag) * GRAVITY_EARTH;
    fusionData.gravity.timestamp = fusionData.gameRotation.timestamp;

    /* Fill Linear Acceleration */
    fusionData.linearAccel.x = fusionData.accel.x - fusionData.gravity.x;
    fusionData.linearAccel.y = fusionData.accel.y - fusionData.gravity.y;
    fusionData.linearAccel.z = fusionData.accel.z - fusionData.gravity.z;
    fusionData.linearAccel.timestamp = fusionData.gameRotation.timestamp;

    return true;
}

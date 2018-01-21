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
#include "GeoMagRotationVector.h"
#include "Quaternion.h"
#include "SensorList.h"

GeoMagRotationVector GeoMagRotationVector::self;

GeoMagRotationVector::GeoMagRotationVector()
{
}

GeoMagRotationVector::~GeoMagRotationVector()
{
}

GeoMagRotationVector *GeoMagRotationVector::getInstance()
{
    return &self;
}

bool GeoMagRotationVector::processFusion(FusionData& fusionData, bool reset)
{
    /* this guarantees that at least n-1 mag samples have been
       processed before reporting that the 6-axis is initialized
    */
    const uint8_t MAG_COUNTS = 10;

    /************************Internal state variables*************************/
    static bool initialized = false;
    static uint8_t mag_cnt = MAG_COUNTS;
    static float nRaw[3];
    static float mRaw[3];
    /*************************************************************************/

    float n[3];
    float m[3];
    float h[3];
    float hNorm;
    float aNorm;
    float r;

    if (reset) {
        initialized = false;
        mag_cnt = MAG_COUNTS;
        return initialized;
    }

    // Get accel data
    n[0] = fusionData.accel.x;
    n[1] = fusionData.accel.y;
    n[2] = fusionData.accel.z;

    // Get mag data
    m[0] = fusionData.mag.x;
    m[1] = fusionData.mag.y;
    m[2] = fusionData.mag.z;

    fusionData.geoMagRotation.timestamp = fusionData.accel.timestamp;

    // Filter the accel and mag for stability.
    if (initialized) {
        mRaw[0] = SIX_AXIS_FILTER_ALPHA * m[0] + (1.f-SIX_AXIS_FILTER_ALPHA) * mRaw[0];
        mRaw[1] = SIX_AXIS_FILTER_ALPHA * m[1] + (1.f-SIX_AXIS_FILTER_ALPHA) * mRaw[1];
        mRaw[2] = SIX_AXIS_FILTER_ALPHA * m[2] + (1.f-SIX_AXIS_FILTER_ALPHA) * mRaw[2];

        nRaw[0] = SIX_AXIS_FILTER_ALPHA * n[0] + (1.f-SIX_AXIS_FILTER_ALPHA) * nRaw[0];
        nRaw[1] = SIX_AXIS_FILTER_ALPHA * n[1] + (1.f-SIX_AXIS_FILTER_ALPHA) * nRaw[1];
        nRaw[2] = SIX_AXIS_FILTER_ALPHA * n[2] + (1.f-SIX_AXIS_FILTER_ALPHA) * nRaw[2];
    } else {
        // Ensure we have new mag samples since reset
        if (mRaw[0] != m[0] || mRaw[1] != m[1] || mRaw[2] != m[2]) {
            if (!(mag_cnt--)) {
                initialized = true;
            }
        }
        mRaw[0] = m[0];
        mRaw[1] = m[1];
        mRaw[2] = m[2];

        nRaw[0] = n[0];
        nRaw[1] = n[1];
        nRaw[2] = n[2];
    }

    // h = mag x accel. h is parallel to ground plane, since <h,accel> == 0.
    // This means h points to magnetic East parallel to the ground.
    Quaternion::cross3( h, mRaw, nRaw );

    // Try to normalize h. If we can't, the device is near free-fall.
    hNorm = sqrtf(h[0] * h[0] + h[1] * h[1] + h[2] * h[2]);
    if (hNorm < 0.1f)
        return initialized;
    h[0] /= hNorm;
    h[1] /= hNorm;
    h[2] /= hNorm;
    // Normalize gravity.
    aNorm = sqrtf(nRaw[0]*nRaw[0]+nRaw[1]*nRaw[1]+nRaw[2]*nRaw[2]);
    n[0] = nRaw[0]/aNorm;
    n[1] = nRaw[1]/aNorm;
    n[2] = nRaw[2]/aNorm;

    // m = accel x h. <m,h> == 0 and <m,accel> == 0.
    // This means m points magnetic north parallel to the ground.
    Quaternion::cross3( m, n, h );

    // Now, we have an orthogonal world-coordinate system (h,m,n):
    // x -> h (east)
    // y -> m (north)
    // z -> n (up)

    // The rotation matrix from device to world coordinates:
    //     [ h[0] h[1] h[2] ]
    // Q = [ m[0] m[1] m[2] ]
    //     [ n[0] n[1] n[2] ]

    // Use numerically-stable algorithm to recover the quaternion from the
    // rotation matrix. I am sorry for all the expensive sqrtf() calls, but
    // that's the price for numerical stability.
    // - http://en.wikipedia.org/wiki/Rotation_matrix#Quaternion
    // NOTE: the fabs() are to prevent NaNs, as occasionally the argument may
    //       evaluate to small negative numbers (-5e-8) due to the nature of
    //       single-precision.
    r = sqrtf(fabs(1.0f + h[0] + m[1] + n[2]));
    fusionData.geoMagRotation.a = copysignf(0.5f*sqrtf(fabs(1.0f+h[0]-m[1]-n[2])), n[1]-m[2]);
    fusionData.geoMagRotation.b = copysignf(0.5f*sqrtf(fabs(1.0f-h[0]+m[1]-n[2])), h[2]-n[0]);
    fusionData.geoMagRotation.c = copysignf(0.5f*sqrtf(fabs(1.0f-h[0]-m[1]+n[2])), m[0]-h[1]);
    fusionData.geoMagRotation.d = 0.5f*r;
    fusionData.geoMagRotation.accuracy = 0;

    // Notice here that fusionData.geoMagRotation.d >= 0 by construction, implying the rotation
    // angle encoded by the quaternion is in [-pi/2, pi/2).

    return initialized;
}

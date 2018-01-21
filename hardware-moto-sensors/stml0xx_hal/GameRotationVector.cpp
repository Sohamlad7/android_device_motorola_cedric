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
#include "GameRotationVector.h"
#include "GyroIntegration.h"
#include "Quaternion.h"
#include "SensorList.h"

GameRotationVector GameRotationVector::self;

GameRotationVector::GameRotationVector()
{
}

GameRotationVector::~GameRotationVector()
{
}

GameRotationVector *GameRotationVector::getInstance()
{
    return &self;
}

int GameRotationVector::permSort(size_t* p, float* a, size_t const len)
{
    int swaps = 0;
    float maxVal;
    size_t i, j, k;
    float _ftmp;
    size_t _itmp;
#define _flt_swap(a,b) \
    do { \
        _ftmp = a; \
        a = b; \
        b = _ftmp; \
    } while (0);
#define _szt_swap(a,b) \
    do { \
        _itmp = a; \
        a = b; \
        b = _itmp; \
    } while (0);

    // Initialize permutation to the identity p[i] = i
    for( i = 0; i < len; ++i )
        p[i] = i;
    // Selection sort, keeping track of permutation
    for(i = 0; i < len; ++i) {
        maxVal = -FLT_MAX;
        k = i;
        for(j = i; j < len; ++j) {
            if( fabs(a[j]) > maxVal )
            {
                k = j;
                maxVal = fabs(a[j]);
            }
        }
        _flt_swap(a[i], a[k]);
        _szt_swap(p[i], p[k]);
        swaps += (i == k) ? 0 : 1;
    }
#undef _szt_swap
#undef _flt_swap

    return (swaps % 2)? -1 : 1;
}

bool GameRotationVector::processFusion(FusionData& fusionData, bool reset)
{
    /************************Internal state variables*************************/
    static struct GyroIntegrationState gis;
    static int initialized;
    static float nRaw[3];
    static float h[3];
    /*************************************************************************/

    float n[3];
    float m[3];
    size_t nPerm[3];

    if (reset) {
        gis.initialized = 0;
        initialized = 0;
    }

    n[0] = fusionData.accel.x;
    n[1] = fusionData.accel.y;
    n[2] = fusionData.accel.z;

    fusionData.gameRotation.timestamp = fusionData.gyro.timestamp;

    // Filter the accel for stability.
    if (initialized) {
        nRaw[0] = SIX_AXIS_FILTER_ALPHA * n[0] + (1.f-SIX_AXIS_FILTER_ALPHA) * nRaw[0];
        nRaw[1] = SIX_AXIS_FILTER_ALPHA * n[1] + (1.f-SIX_AXIS_FILTER_ALPHA) * nRaw[1];
        nRaw[2] = SIX_AXIS_FILTER_ALPHA * n[2] + (1.f-SIX_AXIS_FILTER_ALPHA) * nRaw[2];

    } else {
        nRaw[0] = n[0];
        nRaw[1] = n[1];
        nRaw[2] = n[2];

        // Generate arbitrary vector orthogonal to n. Sort the n values so the
        // division is numerically stable.
        permSort(nPerm, n, 3);
        if (fabs(nRaw[nPerm[0]]) <= 2*FLT_EPSILON)
        {
            ALOGD("GameRotationVector: freefall");
            return false;
        }
        h[nPerm[2]] = 1.f;
        h[nPerm[1]] = 1.f;
        h[nPerm[0]] = -( nRaw[nPerm[2]] + nRaw[nPerm[1]] ) / nRaw[nPerm[0]];

        initialized = 1;
    }

    // Normalize gravity vector
    float tmpNorm = sqrtf(nRaw[0]*nRaw[0] + nRaw[1]*nRaw[1] + nRaw[2]*nRaw[2]);
    if (tmpNorm < 2*FLT_EPSILON)
    {
        ALOGD("GameRotationVector: unrenormalizable gravity");
        return false;
    }

    n[0] = nRaw[0]/tmpNorm;
    n[1] = nRaw[1]/tmpNorm;
    n[2] = nRaw[2]/tmpNorm;

    // Orthogonalize/normalize h ("east") vector
    float projection = h[0] * n[0] + h[1] * n[1] + h[2] * n[2];
    h[0] -= projection * n[0];
    h[1] -= projection * n[1];
    h[2] -= projection * n[2];
    tmpNorm = sqrtf(h[0] * h[0] + h[1] *h[1] + h[2] * h[2]);
    if (tmpNorm < 2*FLT_EPSILON)
    {
        ALOGD("GameRotationVector: unrenormalizable east");
        return false;
    }
    h[0] /= tmpNorm;
    h[1] /= tmpNorm;
    h[2] /= tmpNorm;

    // m = n x h. <m,h> == 0 and <m,n> == 0.
    // This means m points "north" parallel to the ground.
    Quaternion::cross3( m, n, h );

    // Now, we have an orthogonal world-coordinate system (h,m,n):
    // x -> h ("east")
    // y -> m ("north")
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
    float r = sqrtf(fabs(1.0f + h[0] + m[1] + n[2]));
    fusionData.gameRotation.a = copysignf(0.5f * sqrtf(fabs(1.0f + h[0] -m [1] - n[2])), n[1] - m[2]);
    fusionData.gameRotation.b = copysignf(0.5f * sqrtf(fabs(1.0f - h[0] +m [1] - n[2])), h[2] - n[0]);
    fusionData.gameRotation.c = copysignf(0.5f * sqrtf(fabs(1.0f - h[0] -m [1] + n[2])), m[0] - h[1]);
    fusionData.gameRotation.d = 0.5f * r;
    fusionData.gameRotation.accuracy = 0;

    GyroIntegration::integrate( &gis, fusionData.gameRotation, fusionData.gameRotation, fusionData );

    // Notice here that quatGame[3] >= 0 by construction, implying the rotation
    // angle encoded by the quaternion is in [-pi/2, pi/2).

    // See the documentation in the linear accel/gravity vectors to
    // understand what this does. Basically, embed the x "east" vector
    // into a quaternion and extract it from quatGame. This is how we keep
    // the h vector up-to-date using gyro integration
    //
    // qEast = quatGame^{-1} * qx * quatGame
    QuatData qx = {1.f, 0.f, 0.f, 0.f, 0.f, 0};
    QuatData qEast = {0.f, 0.f, 0.f, 0.f, 0.f, 0};

    Quaternion::inv( qEast, fusionData.gameRotation );
    Quaternion::mul( qx, qx, fusionData.gameRotation );
    Quaternion::mul_noRenormalize(qEast, qEast, qx);

    h[0] = qEast.a;
    h[1] = qEast.b;
    h[2] = qEast.c;

    return true;
}

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

#ifndef GYRO_INTEGRATION_H
#define GYRO_INTEGRATION_H

#include <stdint.h>

#include "FusionSensorBase.h"

/*
 * \brief State to be passed into \c gyroIntegration()
 *
 * \see \c gyroIntegration()
 */
struct GyroIntegrationState
{
    int initialized;
    QuatData quatGyro;
};

class GyroIntegration {
public:
    /*
     * \brief Use gyro to integrate a rotation vector forward
     *
     * Memoryless (stateless, amnesiatic...)  function to integrate rvIn with
     * gyro to produce rvOut.
     *
     * \see \c GyroIntegrationState
     *
     * \param[inout] gis state of the function
     * \param[out] rvOut output integrated rotation vector
     * \param[in] rvIn input rotation vector to integrate
     * \param[in] rvGyroDelta if not null, stores the incremental gyro rotation
     * \param[in] timeStamp gyro data timestamp
     *      vector
     */

    static void integrate(
        struct GyroIntegrationState* gis,
        QuatData& rvOut,
        const QuatData& rvIn,
        FusionData& fusionData
    );
};

#endif // GYRO_INTEGRATION_H

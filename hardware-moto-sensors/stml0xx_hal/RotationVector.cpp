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
#include "GyroIntegration.h"
#include "Quaternion.h"
#include "RotationVector.h"
#include "SensorList.h"

RotationVector RotationVector::self;

RotationVector::RotationVector()
{
}

RotationVector::~RotationVector()
{
}

RotationVector *RotationVector::getInstance()
{
    return &self;
}

bool RotationVector::processFusion(FusionData& fusionData, bool reset)
{
    static struct GyroIntegrationState gis;

    if (reset) {
        gis.initialized = 0;
	}

    // Integrate forward the 6-axis
    GyroIntegration::integrate(&gis, fusionData.rotationVector, fusionData.geoMagRotation, fusionData);
    fusionData.rotationVector.accuracy = 0;
    fusionData.rotationVector.timestamp = fusionData.gyro.timestamp;

    return true;
}

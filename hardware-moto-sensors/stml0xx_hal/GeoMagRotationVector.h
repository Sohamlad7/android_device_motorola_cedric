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

#ifndef GEOMAG_ROTATION_VECTOR_H
#define GEOMAG_ROTATION_VECTOR_H

#include <stdint.h>
#include "FusionSensorBase.h"

class GeoMagRotationVector : public FusionSensorBase {
public:
    GeoMagRotationVector();
    virtual ~GeoMagRotationVector();

    static GeoMagRotationVector* getInstance();

    bool processFusion(FusionData& fusionData, bool reset);

private:
    static GeoMagRotationVector self;
};

#endif // GEOMAG_ROTATION_VECTOR_H

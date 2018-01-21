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

#ifndef GAME_ROTATION_VECTOR_H
#define GAME_ROTATION_VECTOR_H

#include <stdint.h>
#include "FusionSensorBase.h"

class GameRotationVector : public FusionSensorBase {
public:
    GameRotationVector();
    virtual ~GameRotationVector();

    static GameRotationVector* getInstance();

    /*!
     * \brief Process accel and gyro samples to compute game rotation vector
     * \param[out] fusionData Stores computed data into fusionData.gameRotation as follows:
     *  fusionData.gameRotation.a: Rotation vector component along the x axis (x * sin(θ/2))
     *  fusionData.gameRotation.b: Rotation vector component along the y axis (y * sin(θ/2))
     *  fusionData.gameRotation.c: Rotation vector component along the z axis (z * sin(θ/2))
     *  fusionData.gameRotation.d: Scalar component of the rotation vector ((cos(θ/2))
     *  fusionData.gameRotation.accuracy: 0, per https://source.android.com/devices/sensors/sensor-types.html#game_rotation_vector
     * \param[in] reset if true, re-initialize fusion algorithm
     * \returns true if fusion data is ready, false if not
     */
    bool processFusion(FusionData& fusionData, bool reset);

private:
    static GameRotationVector self;

    /*!
     * \brief Sort by absolute value in descending order with permutation
     *
     * - Memory Complexity: O(1)
     * - Time Complexity: O(len^2)
     *
     * Probably should be moved somewhere else you want sorting with permutations.
     *
     * \param[out] p array of size \c len storing sorting permutation
     * \param[inout] a array of size \c len to be sorted
     * \param[in] len size of arrays \c p and \c a
     * \returns the sign of the permutation (either -1 or 1)
     */
    static int permSort(size_t* p, float* a, size_t const len);
};

#endif // GAME_ROTATION_VECTOR_H

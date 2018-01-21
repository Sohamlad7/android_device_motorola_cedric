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

#ifndef FUSION_SENSOR_BASE_H
#define FUSION_SENSOR_BASE_H

#include <stdint.h>

/*!
 * \brief Cartesian data
 *
 * Sensor data represented in cartesion coordinates with
 * a timestamp of when the sample was taken.
 */
typedef struct {
    float x;
    float y;
    float z;
    int64_t timestamp;
} CartData;

/*!
 * \brief Quaternion data
 *
 * Sensor fusion data represented in quaternians with
 * a timestamp of when the newest sensor data used in
 * the fusion was taken.
 */
typedef struct {
    float a;
    float b;
    float c;
    float d;
    float accuracy;
    int64_t timestamp;
} QuatData;

/*!
 * \brief Latest sensor data
 *
 * The newest sensor and sensor fusion readings.
 * These values are filled in in the HubSensors code and used
 * for sensor fusion.
 */
typedef struct {
    CartData accel;
    CartData gyro;
    CartData mag;
    QuatData gameRotation;
    QuatData geoMagRotation;
    QuatData rotationVector;
    CartData gravity;
    CartData linearAccel;
} FusionData;

/******************************************************************************/
/*    Rotation Vector Tuning                                                  */
/******************************************************************************/

/*!
 * \brief Normalized LP filter constant for the 6-axis RV
 *
 * Acceptable range for this parameter is (0,1).
 *
 * The cutoff frequency is
 * \f[
 *    f_c = \alpha \frac{f_s}{2},
 * \f]
 * where \f$ f_s \f$ is the sampling frequency (in Hz).
 *
 * This should be chosen to balance noise and responsiveness.
 */
#define SIX_AXIS_FILTER_ALPHA (0.016f)

/*!
 * \brief Normalized LP/HP filter constant for the 9-axis RV fusion
 *
 * Acceptable range for this parameter is (0,1).
 *
 * We use this alpha to keep the low frequencies of the 6-axis RV, and the high
 * frequencies of the gyro. This constant should be "long" with respect to
 * \c SIX_AXIS_FILTER_ALPHA so that there is no noticeable lag from the 6-axis.
 * However, the longer it is, the more a gyro bias will pull the RV off.
 *
 * \see SIX_AXIS_FILTER_ALPHA
 */
#define NINE_AXIS_FILTER_ALPHA_FAST (0.000f)
#define NINE_AXIS_FILTER_ALPHA_SLOW (0.003f)

/*!
 * \brief Normalized LP/HP filter constant for the 9-axis RV fusion
 *
 * Gyro magnitudes above this threshold will switch to the NINE_AXIS_FILTER_ALPHA_FAST
 * and otherwise use the NINE_AXIS_FILTER_ALPHA_SLOW wfor the 9-axis fusion
 *
 */
#define NINE_AXIS_FILTER_SELECT_THRESH (0.5f)


class FusionSensorBase {
public:
    FusionSensorBase() {};
    virtual ~FusionSensorBase() {};

    /*!
     * \brief Process fusion using current sensor data
     *
     * \param[out] fusionData Computed fusion data
     * \param[in] reset if true, re-initialize fusion algorithm
     * \returns true if fusion data is ready, false if not
     */
    virtual bool processFusion(FusionData& fusionData, bool reset) = 0;

protected:
};

#endif // FUSION_SENSOR_BASE_H

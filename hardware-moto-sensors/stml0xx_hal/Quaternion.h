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

#ifndef QUATERNION_H
#define QUATERNION_H

#include <stdint.h>

#include "FusionSensorBase.h"

/*!
 * \page pageQuaternions Quaternions
 *
 * \section secQuatIntro Quick Introduction
 *
 * All the rotation vector functionality directly uses quaternions to represent
 * rotations and orientations. A quaternion is simply an extension of complex
 * numbers with two additional imaginary units (j and k). For example,
 * \f[
 *   q := a*i + b*j + c*k + w,
 * \f]
 * where
 * \f[
 *   i^2 = j^2 = k^2 = ijk = -1.
 * \f]
 * The above multiplicative rule defines a field over such quaternions such that they can be
 * multiplied and added in a sensible way.
 *
 * \section secQuatRotations Quaternions to Describe 3D Rotations
 *
 * Quaternions can be used to describe rotations. A rotation of \f$ \theta \f$
 * radians about the unit vector [x;y;z] is encoded as
 * \f[
 *   q = [x \sin(\theta/2)] i + [y \sin(\theta/2)] j + [z \sin(\theta/2)] k + \cos(\theta/2),
 * \f]
 * by extending Euler's identity from complex numbers to quaternions. Multiplying
 * quaternions, like
 * \f[
 *   q = q_1 q_2,
 * \f]
 * means \f$ q \f$ is a new rotation that is obtained by first performing
 * rotation q_1, then rotation q_2. Notice that this is NOT the same thing as
 * first rotating by q_2, then by q_1. So, by extending complex numbers from 2
 * to four dimensions, we have lost commutativity and must be careful of the
 * order of multiplication.
 *
 * When using quaternions to describe rotations, the magnitude has no meaning,
 * so we restrict all rotation quaternions to be on the unit 4D hypersphere:
 * \f[
 *   ||q||^2 = a^2 + b^2 + c^2 + d^2 = 1.
 * \f]
 *
 * One more caveat is to note that it matters how you choose to restrict \f$ \theta \f$.
 * Of course you can choose any \f$ \pi \f$ range of values.
 * A typical choice is \f$ \theta \in [-\pi/2,pi/2) \f$ so that \f$ w >= 0 \f$
 * for all rotations. So, whatever manipulations you do, you must always ensure
 * that your assumptions about the range of \f$ \theta \f$ are consistent.
 *
 * \section secQuatReading Reading Material
 *
 * - [Wikipedia: Quaternions](http://en.wikipedia.org/wiki/Quaternion)
 * - [Wikipedia: Rotation Matrix](http://en.wikipedia.org/wiki/Rotation_matrix)
 */

class Quaternion {
public:
    /*!
     * \brief out = u X v (in 3 dimensions)
     *
     * All parameters are length 3.
     * \note{\c out must NOT be aliased with \c u or \c v}
     *
     * \param[out] out output of cross product
     * \param[in] u left parameter to cross product
     * \param[in] v right parameter to cross product
     */
    static void cross3(float* out, float* u, float* v);

    /*!
      * \brief out = q^(-1)
     *
     * Produces the inverse quaternion, i.e.
     * \f[
     *   qq^{-1} = q^{-1}q = 1
     * \f]
     *
     * \param[out] out inverse of \c q
     * \param[in] q quaternion to invert
    */
    static void inv(QuatData& out, const QuatData& q);

    /*!
     * \brief q = q1*q2
     *
     * The result is automatically normalized.
     * Quaternion multiplication is not commutative, i.e.
     * \f[
     *    q1*q2 \neq q2*q1.
     * \f]
     *
     * \note{It is OK if \c q is aliased with \c q1 or \c q2.}
     *
     * \see renormalize()
     * \returns 0 on success, 1 on failure
    */
    static int mul(QuatData& q, const QuatData& q1, const QuatData& q2);

    /*!
     * \brief Renormalize quaternion if needed
     *
     * If \f$ ||q|| \f$ is not a unit vector, make it one.
     * Also force q[3] >= 0 so that the encoded angle is in \f$ [-pi/2, pi/2) \f$.
     *
     * \returns 0 on success, 1 if \c q is non-renormalizable
    */
    static int renormalize(QuatData& q);

    /*!
     * \brief q = q1*q2
     *
     * Same as \c mul() without normalization.
     *
     * \see mul()
    */
    static void mul_noRenormalize(QuatData& q, const QuatData& q1, const QuatData& q2);

    //! \brief Squared distance between two quaternions
    static float dist(const QuatData& q1, const QuatData& q2);

    /*!
     * \brief out = renormalize(alpha*q1 + (1-alpha)*q2)
     *
     * Forms a linear interpolant of q1 and q2 along the shortest path from q1 to
     * q2, and renormalizes.
     *
     * There are other ways to interpolate quaternions, e.g.
     * SLERP (spherical-linear), but those are more complicated, computationally
     * expensive, and the comparative advantages (constant angular speed) small.
     *
     * \note{It is OK if \c out is aliased with \c q1 or \c q2.}
     *
     * \param out[out] Resultant quaternion (normalized)
     * \param q1[in] First input to interpolation
     * \param q2[in] Second input to interpolation
     * \param alpha[in] Interpolation factor in [0,1]
     *
     * \returns 0 on success, 1 on failure
    */
    static int linInterp(QuatData& out, const QuatData& q1, const QuatData& q2, const float alpha);
};

#endif // QUATERNION_H

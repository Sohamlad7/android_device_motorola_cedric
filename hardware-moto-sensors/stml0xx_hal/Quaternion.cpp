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

#include <math.h>
#include <float.h>
#include "Quaternion.h"

void Quaternion::cross3(float* out, float* u, float* v)
{
    out[0] = u[1]*v[2] - u[2]*v[1];
    out[1] = u[2]*v[0] - u[0]*v[2];
    out[2] = u[0]*v[1] - u[1]*v[0];
}

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
void Quaternion::inv(QuatData& out, const QuatData& q)
{
    out.a = -q.a;
    out.b = -q.b;
    out.c = -q.c;
    out.d = q.d;
}

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
int Quaternion::mul(QuatData& q, const QuatData& q1, const QuatData& q2)
{
    QuatData out;
    int ret;

    out.a = q1.d * q2.a + q1.a * q2.d + q1.b * q2.c - q1.c * q2.b;
    out.b = q1.d * q2.b - q1.a * q2.c + q1.b * q2.d + q1.c * q2.a;
    out.c = q1.d * q2.c + q1.a * q2.b - q1.b * q2.a + q1.c * q2.d;
    out.d = q1.d * q2.d - q1.a * q2.a - q1.b * q2.b - q1.c * q2.c;
    ret = renormalize(out);

    q.a = out.a;
    q.b = out.b;
    q.c = out.c;
    q.d = out.d;

    return ret;
}

/*!
 * \brief Renormalize quaternion if needed
 *
 * If \f$ ||q|| \f$ is not a unit vector, make it one.
 * Also force q[3] >= 0 so that the encoded angle is in \f$ [-pi/2, pi/2) \f$.
 *
 * \returns 0 on success, 1 if \c q is non-renormalizable
 */
int Quaternion::renormalize(QuatData& q)
{
    // Square magnitude
    float mag = q.a * q.a + q.b * q.b + q.c * q.c + q.d * q.d;

#define MAG_TOL (0.0001f)
    if (mag < MAG_TOL) {
        // This is bad. Quaternion is not renormalizable.
        q.a = 0.f;
        q.b = 0.f;
        q.c = 0.f;
        q.d = 1.f;
        return 1;
    } else if (mag > 1.f + MAG_TOL || mag < 1.f - MAG_TOL) {
        // Not only do we want to normalize, but we want to keep
        // q[3] >= 0 so that the encoded angle is in [-pi/2, pi/2)
        mag = copysignf( sqrtf(mag), q.d );
        q.a /= mag;
        q.b /= mag;
        q.c /= mag;
        q.d /= mag;
    }
#undef MAG_TOL

   return 0;
}

/*!
 * \brief q = q1*q2
 *
 * Same as \c mul() without normalization.
 * 
 * \see mul()
 */
void Quaternion::mul_noRenormalize(QuatData& q, const QuatData& q1, const QuatData& q2)
{
    QuatData out;

    out.a = q1.d * q2.a + q1.a * q2.d + q1.b * q2.c - q1.c * q2.b;
    out.b = q1.d * q2.b - q1.a * q2.c + q1.b * q2.d + q1.c * q2.a;
    out.c = q1.d * q2.c + q1.a * q2.b - q1.b * q2.a + q1.c * q2.d;
    out.d = q1.d * q2.d - q1.a * q2.a - q1.b * q2.b - q1.c * q2.c;

    q.a = out.a;
    q.b = out.b;
    q.c = out.c;
    q.d = out.d;
}

//! \brief Squared distance between two quaternions
float Quaternion::dist(const QuatData& q1, const QuatData& q2)
{
    return (q1.a - q2.a) * (q1.a - q2.a)
         + (q1.b - q2.b) * (q1.b - q2.b)
         + (q1.c - q2.c) * (q1.c - q2.c)
         + (q1.d - q2.d) * (q1.d - q2.d);
}

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
int Quaternion::linInterp(QuatData& out, const QuatData& q1, const QuatData& q2, const float alpha)
{
    QuatData minusQ2;
    float oneMinusAlpha = 1.f-alpha;

    // There are always 2 ways to get from q1 to q2, just like there are always
    // 2 paths between any two places on Earth (the short arc, and the long arc).
    // We will test to see which way is shorter by constructing -q2.
    minusQ2.a = -q2.a;
    minusQ2.b = -q2.b;
    minusQ2.c = -q2.c;
    minusQ2.d = -q2.d;

    // Looks like the opposite way will be shorter...
    if ( dist(q1, minusQ2) < dist(q1, q2) )
            oneMinusAlpha = -oneMinusAlpha;

    out.a = alpha * q1.a + oneMinusAlpha * q2.a;
    out.b = alpha * q1.b + oneMinusAlpha * q2.b;
    out.c = alpha * q1.c + oneMinusAlpha * q2.c;
    out.d = alpha * q1.d + oneMinusAlpha * q2.d;

    return renormalize(out);
}

/******************************************************************************
 *
 *  $Id: AKDirection6D.h 255 2014-06-12 06:04:43Z yamada.rj $
 *
 * -- Copyright Notice --
 *
 * Copyright (c) 2004 Asahi Kasei Microdevices Corporation, Japan
 * All Rights Reserved.
 *
 * This software program is the proprietary program of Asahi Kasei Microdevices
 * Corporation("AKM") licensed to authorized Licensee under the respective
 * agreement between the Licensee and AKM only for use with AKM's electronic
 * compass IC.
 *
 * THIS SOFTWARE IS PROVIDED TO YOU "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABLITY, FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT OF
 * THIRD PARTY RIGHTS, AND WE SHALL NOT BE LIABLE FOR ANY LOSSES AND DAMAGES
 * WHICH MAY OCCUR THROUGH USE OF THIS SOFTWARE.
 *
 * -- End Asahi Kasei Microdevices Copyright Notice --
 *
 ******************************************************************************/
#ifndef AKSC_INC_AKDIRECTION6D_H
#define AKSC_INC_AKDIRECTION6D_H

#include "AKMDevice.h"


//========================= Constant definition =========================//

//========================= Type declaration  ===========================//

//========================= Prototype of Function =======================//
AKLIB_C_API_START
int16 AKSC_VNorm(
    const int16vec *v,          //(i)   : Vector
    const int16vec *o,          //(i)   : Offset
    const int16vec *s,          //(i)   : Amplitude
    const int16    tgt,         //(i)   : Target sensitivity value
    int16vec       *nv          //(o)   : Resulted normalized vector
);

void AKSC_SetLayout(            //      :
    int16vec        *v,         //(i/o) : Magnetic/Gravity vector data
    const I16MATRIX *layout     //(i)   : Layout matrix
);

#ifndef AKSC_DISABLE_D6D
int16 AKSC_DirectionS3(
    const uint8     licenser[], //(i)   : Licenser
    const uint8     licensee[], //(i)   : Licensee
    const int16     key[],      //(i)   : Key
    const int16vec  *h,         //(i)   : Geomagnetic vector (offset and sensitivity are  compensated)
    const int16vec  *a,         //(i)   : Acceleration vector (offset and sensitivity are  compensated)
    const int16vec  *dvec,      //(i)   : A vector to define reference axis of the azimuth on the terminal coordinate system
    const I16MATRIX *hlayout,   //(i)   : Layout matrix for geomagnetic vector
    const I16MATRIX *alayout,   //(i)   : Layout matrix for acceleration vector
    int16           *theta,     //(o)   : Azimuth direction (degree)
    int16           *delta,     //(o)   : The inclination (degree)
    int16           *hr,        //(o)   : Geomagnetic vector size
    int16           *hrhoriz,   //(o)   : Horizontal element of geomagnetic vector
    int16           *ar,        //(o)   : Acceleration vector size
    int16           *phi180,    //(o)   : Pitch angle (-180 to +180 degree)
    int16           *phi90,     //(o)   : Pitch angle (-90 to +90 degree)
    int16           *eta180,    //(o)   : Roll angle  (-180 to +180 degree)
    int16           *eta90,     //(o)   : Roll angle  (-90 to +90 degree)
    I16MATRIX       *mat,       //(o)   : Rotation matrix
    I16QUAT         *quat       //(o)   : Rotation Quaternion
);

int16 AKSC_ThetaFilter(         //(o)    : theta filterd
    const int16 the,            //(i)   : current theta(Q6)
    const int16 pre_the,        //(i)   : previous theta(Q6)
    const int16 scale           //(i)   : Q12.
);
#endif
AKLIB_C_API_END

#endif


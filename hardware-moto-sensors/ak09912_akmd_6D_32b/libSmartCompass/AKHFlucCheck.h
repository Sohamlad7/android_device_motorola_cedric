/******************************************************************************
 *
 *  $Id: AKHFlucCheck.h 255 2014-06-12 06:04:43Z yamada.rj $
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
#ifndef AKSC_INC_AKHFLUCCHECK_H
#define AKSC_INC_AKHFLUCCHECK_H

#include "AKMDevice.h"

//========================= Constant definition =========================//

//========================= Type declaration  ===========================//
typedef struct _AKSC_HFLUCVAR {
    int16vec href;             // Basis of magnetic field
    int16    th;               // The range of fluctuation
} AKSC_HFLUCVAR;

//========================= Prototype of Function =======================//
AKLIB_C_API_START

int16 AKSC_InitHFlucCheck(
    AKSC_HFLUCVAR  *hflucv,    //(o)   : A set of criteria to be initialized
    const int16vec *href,      //(i)   : Initial value of basis
    const int16    th          //(i)   : The range of fluctuation
);

int16 AKSC_HFlucCheck(
    AKSC_HFLUCVAR  *hflucv,    //(i/o) : A set of criteria
    const int16vec *hdata      //(i)   : Current magnetic vector
);

void AKSC_TransByHbase(
    const int32vec *prevHbase, //(i)   : Previous hbase
    const int32vec *hbase,     //(i)   : Current hbase
    int16vec       *ho,        //(i/o) : Offset value based on current hbase(16bit)
    int32vec       *ho32,      //(i/o) : Offset value based on current hbase(32bit)
    int16          *overflow   //(o)   : 0;success, 1;ho overflow.
);

AKLIB_C_API_END

#endif

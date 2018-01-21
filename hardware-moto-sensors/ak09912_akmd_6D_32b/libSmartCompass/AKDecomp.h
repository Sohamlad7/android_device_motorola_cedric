/******************************************************************************
 *
 *  $Id: AKDecomp.h 255 2014-06-12 06:04:43Z yamada.rj $
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
#ifndef AKSC_INC_AKDECOMP_H
#define AKSC_INC_AKDECOMP_H

#include "AKMDevice.h"

//========================= Type declaration  ===========================//

//========================= Constant definition =========================//
#define AKSC_HSENSE_TARGET  833  // Target sensitivity for magnetic sensor

//========================= Prototype of Function =======================//
AKLIB_C_API_START

void AKSC_InitDecomp(
    int16vec hdata[]             //(i/o) : Magnetic data. index 0 is earlier data. The size of hdata must be HDATA_SIZE.
);

int16 AKSC_DecompS3(             //(o)   : (0)abend, (1)normally calculated
    const int16    device,       //(i)   : Device part number (4 digits)
    const int16    bdata[],      //(i)   : Block data
    const int16    hNave,        //(i)   : The number of magnetic data to be averaged. hNave must be 0,1,2,4,8,16,32
    const int16vec *asa,         //(i)   : Sensitivity adjustment value(the value read from Fuse ROM)
    const uint8    *pdc,         //(i)   : Reserved for debug operation.
    int16vec       hdata[],      //(i/o) : Magnetic data. index 0 is earlier data.
    int32vec       *hbase,       //(i/o) : Magnetic data base.
    int16          *hn,          //(o)   : The number of magnetic data, output 1.
    int16vec       *have,        //(o)   : Average of magnetic data
    int16          *temperature, //(o): Temperature in celcius degree.
    int16          *dor,         //(o)   : 0;normal data, 1;data overrun is occurred.
    int16          *derr,        //(o)   : 0;normal data, 1;data read error occurred.
    int16          *hofl,        //(o)   : 0;normal data, 1;data overflow occurred.
    int16          *cb,          //(o)   : 0;hbase is not changed, 1;hbase is changed.
    int16          *dc           //(o)   : Reserved for debug operation.
);

AKLIB_C_API_END

#endif

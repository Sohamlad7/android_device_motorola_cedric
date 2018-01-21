/******************************************************************************
 *
 *  $Id: AKHDOE.h 255 2014-06-12 06:04:43Z yamada.rj $
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
#ifndef AKSC_INC_AKHDOE_H
#define AKSC_INC_AKHDOE_H

#include "AKMDevice.h"

//========================= Constant definition =========================//
#define AKSC_NUM_HCOND          2       // The number of HCOND. e.g. HCOND[0];for small magnetism size, HCOND[1];for normal magnetism size
#define AKSC_MAX_HDOE_LEVEL     3       // The maximum DOE level
#define AKSC_HBUF_SIZE          32      // Buffer size for DOE
#define AKSC_HOBUF_SIZE         16      // Offset buffer size for DOE

//========================= Macro definition =========================//
#define AKSC_InitHDOEProcPrmsS3(hdoev, idxhcond, ho, hdst) zzAKSC_InitHDOEProcPrmsS3((hdoev), 0, 0, (idxhcond), (ho), (hdst))
#define AKSC_HDOEProcessS3(licenser, licensee, key, hdoev, hdata, hn, ho, hdst) zzAKSC_HDOEProcessS3((licenser), (licensee), (key), (hdoev), (hdata), (hn), 0, 1, (ho), (hdst))

//========================= Type declaration  ===========================//
typedef enum _AKSC_HDFI {
    AKSC_HDFI_SMA       = 0,           // Low magnetic intensity
    AKSC_HDFI_NOR       = 1            // Normal magnetic intensity
} AKSC_HDFI;

typedef enum _AKSC_HDST {
    AKSC_HDST_UNSOLVED  = 0,           // Offset is not determined.
    AKSC_HDST_L0        = 1,           // Offset has been determined once or more with Level0 parameter
    AKSC_HDST_L1        = 2,           // Offset has been determined once or more with Level1 parameter
    AKSC_HDST_L2        = 3            // Offset has been determined once or more with Level2 parameter
} AKSC_HDST;

typedef struct _AKSC_HDOEVAR {         // Variables necessary for DOE calculation
    void     **HTH;                    // [AKSC_NUM_HCOND][AKSC_MAX_HDOE_LEVEL]
    int16vec hbuf[AKSC_HBUF_SIZE];     // Buffer for measurement values
    int16vec hobuf[AKSC_HOBUF_SIZE];   // Buffer for offsets
    int16vec hvobuf[AKSC_HOBUF_SIZE];  // Buffer for successful offsets
    int16    hdoeCnt;                  // DOE counter
    int16    hdoeLv;                   // HDOE level
    int16    hrdoe;                    // Geomagnetic vector size
    int16    hthIdx;                   // Index of HTH. This value can be from 0 to AKSC_NUM_HCOND-1

    void     **HTHHR;                  // [AKSC_NUM_HCOND or 1][AKSC_MAX_HDOE_LEVEL or 1]
    int16vec hobufHR[AKSC_HOBUF_SIZE]; // Buffer for offsets to check the size of geomagnetism
    int16    hrdoeHR;

    int16    reserved;                 // Reserve
} AKSC_HDOEVAR;

//========================= Prototype of Function =======================//
AKLIB_C_API_START
void zzAKSC_InitHDOEProcPrmsS3(
    AKSC_HDOEVAR    *hdoev,            //(i/o) : A set of variables
    void            **HTH,             //(i)   : Only 0 is acceptable
    void            **HTHHR,           //(i)   : Only 0 is acceptable
    const int16     idxhcond,          //(i)   : Initial index of criteria
    const int16vec  *ho,               //(i)   : Initial offset
    const AKSC_HDST hdst               //(i)   : Initial DOE level
);

int16 zzAKSC_HDOEProcessS3(            //(o)   : Estimation success(1 ~ hn), failure(0)
    const uint8    licenser[],         //(i)   : Licenser
    const uint8    licensee[],         //(i)   : Licensee
    const int16    key[],              //(i)   : Key
    AKSC_HDOEVAR   *hdoev,             //(i/o) : A set of variables
    const int16vec hdata[],            //(i)   : Vectors of data
    const int16    hn,                 //(i)   : The number of data (the value must be less than the size of hdata array)
    const int16    linkHthHRIdxToHth,  //(i) : (1)link hthHR[idx][] index to hth[idx][]. (0) use only hthHR[0][].
    const int16    linkHthHRLvlToHth,  //(i) : (1)link hthHR[][lvl] level to hth[][lvl]. (0) use only hthHR[][0].
    int16vec       *ho,                //(i/o) : Offset
    AKSC_HDST      *hdst               //(o)   : HDOE status
);

void AKSC_SetHDOELevel(
    AKSC_HDOEVAR    *hdoev,            //(i/o) : A set of variables
    const int16vec  *ho,               //(i)   : Initial offset
    const AKSC_HDST hdst,              //(i)   : DOE level
    const int16     initBuffer         //(i)   : If this flag is 0, don't clear buffer
);
AKLIB_C_API_END

#endif


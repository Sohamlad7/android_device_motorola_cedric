/******************************************************************************
 *
 *  $Id: AKConfigure.h 255 2014-06-12 06:04:43Z yamada.rj $
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
#ifndef AKSC_INC_AKCONFIG_H
#define AKSC_INC_AKCONFIG_H

//========================= Language configuration ===================//
#if defined(__cplusplus)
#define AKLIB_C_API_START   extern "C" {
#define AKLIB_C_API_END     }
#else
#define AKLIB_C_API_START
#define AKLIB_C_API_END
#endif

//========================= Debug configuration ======================//
#if defined(_DEBUG) || defined(DEBUG)
#define AKSC_TEST_MODE
#endif
#if defined(AKSC_TEST_MODE)
#define STATIC
#else
#define STATIC  static
#endif

//========================= Arithmetic Cast ==========================//
#define AKSC_ARITHMETIC_CAST

//======================== Arithmetic configuration ==================//
// If this definition is uncommented, double type is used for
// mathematical functions and AK's floating point type "AKSC_FLOAT".
// Otherwise float type is used for theirs.
// This configuration does not changes floating point calculation used
// in AK's functions. This is intended to cut down memory.
//#define AKSC_MATH_DOUBLE

//======================== Constant value configuration ==============//
// If this definition is uncommented, use FLT_EPSILON, which is
// defined in <float.h>. Otherwise AK's defined value or 0 is used.
#define AKSC_USE_STD_FLOAT

//======================== Use AKM's type definition =================//
// If this definition is uncommented, use type definition, which is
// defined in <stdint.h>. Otherwise AK's definition is used
//#define AKSC_USE_STD_TYPES

// When AK's definition is used (i.e. AKSC_USE_STD_TYPES is commented
// out), int32 type depends on the system architecture. If the system
// is 64bit architecture, please uncomment below.
#define AKSC_ARCH_64BIT

#endif


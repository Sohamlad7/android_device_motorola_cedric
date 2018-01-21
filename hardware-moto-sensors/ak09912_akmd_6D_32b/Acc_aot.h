/******************************************************************************
 *
 *  $Id: Acc_aot.h 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#ifndef AKMD_INC_ACCAOT_H
#define AKMD_INC_ACCAOT_H

#include "AKMD_Driver.h"

/*** Constant definition ******************************************************/

/*** Type declaration *********************************************************/

/*** Global variables *********************************************************/

/*** Prototype of function ****************************************************/
int16_t AOT_InitDevice(void);
void	AOT_DeinitDevice(void);
int16_t AOT_SetEnable(const int8_t enabled);
int16_t AOT_SetDelay(const int64_t ns);
int16_t AOT_GetAccData(int16_t data[3]);
int16_t AOT_GetAccOffset(int16_t offset[3]);
void	AOT_GetAccVector(const int16_t data[3], const int16_t offset[3], int16_t vec[3]);

#endif //AKMD_INC_ACCAOT_H


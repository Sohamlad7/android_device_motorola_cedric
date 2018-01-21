/******************************************************************************
 *
 * $Id: misc.h 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#ifndef AKMD_INC_MISC_H
#define AKMD_INC_MISC_H

#include "AKCompass.h"

/*** Constant definition ******************************************************/
#define AKMD_FORM0	0
#define AKMD_FORM1	1

/*** Type declaration *********************************************************/
typedef struct _AKMD_LOOP_TIME {
	int64_t interval; /*!< Interval of each event */
	int64_t duration; /*!< duration to the next event */
} AKMD_LOOP_TIME;

/*** Global variables *********************************************************/

/*** Prototype of Function  ***************************************************/
void msleep(signed int msec);

int16 misc_openForm(void);
void misc_closeForm(void);
int16 misc_checkForm(void);

struct timespec int64_to_timespec(int64_t val);
int64_t timespec_to_int64(struct timespec* val);
int64_t CalcDuration(struct timespec* begin, struct timespec* end);

int openInputDevice(const char* name);
int16 GetHDOEDecimator(int64_t* time, int16* hdoe_interval);

int16 ConvertCoordinate(
	const	AKMD_PATNO	pat,/*!< [in]  Convert Pattern Number */
			int16vec*	vec	/*!< [out] Coordinate system after converted */
);

#endif /*AKMD_INC_MISC_H*/


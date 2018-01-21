/******************************************************************************
 *
 * $Id: DispMessage.h 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#ifndef AKMD_INC_DISPMESG_H
#define AKMD_INC_DISPMESG_H

// Include file for SmartCompass Library.
#include "AKCompass.h"

/*** Constant definition ******************************************************/
#define DISP_CONV_AKSCF(val)	((val)*0.06f)
#define DISP_CONV_Q6F(val)		((val)*0.015625f)

/*** Type declaration *********************************************************/

/*! These defined types represents the current mode. */
typedef enum _MODE {
	MODE_ERROR,					/*!< Error */
	MODE_FST,					/*!< On board function test */
	MODE_MeasureSNG,			/*!< Measurement */
	MODE_OffsetCalibration,		/*!< Offset calibration */
	MODE_Quit					/*!< Quit */
} MODE;

/*** Prototype of function ****************************************************/
// Disp_   : Display messages.
// Menu_   : Display menu (two or more selection) and wait for user input.

void Disp_StartMessage(void);

void Disp_EndMessage(int ret);

void Disp_MeasurementResult(AKSCPRMS* prms);

// Defined in main.c
void Disp_MeasurementResultHook(AKSCPRMS* prms, const uint16 flag, uint8 *time);

MODE Menu_Main(void);

#endif


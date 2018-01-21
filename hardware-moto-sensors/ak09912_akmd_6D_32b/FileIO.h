/******************************************************************************
 *
 * $Id: FileIO.h 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#ifndef AKMD_INC_FILEIO_H
#define AKMD_INC_FILEIO_H

// Common include files.
#include "AKCommon.h"

// Include file for SmartCompass library.
#include "AKCompass.h"

/*** Constant definition ******************************************************/
#define KEYNAME_SIZE	64
#define HEADER_SIZE     256
#define DELIMITER		" = "

/*** Type declaration *********************************************************/

/*** Global variables *********************************************************/

/*** Prototype of function ****************************************************/
int16 LoadParameters(AKSCPRMS* prms);

int16 LoadPDC(AKSCPRMS* prms);

int16 LoadInt(
	FILE* lpFile,
	const char* lpKeyName,
	int* val
);

int16 LoadInt16vec(
	FILE* lpFile,
	const char* lpKeyName,
	int16vec* vec
);

int16 LoadInt32vec(
	FILE* lpFile,
	const char* lpKeyName,
	int32vec* vec
);

int16 SaveParameters(AKSCPRMS* prms);

int16 SaveInt(
	FILE* lpFile,
	const char* lpKeyName,
	const int val
);

int16 SaveInt16vec(
	FILE* lpFile,
	const char* lpKeyName,
	const int16vec* vec
);

int16 SaveInt32vec(
	FILE* lpFile,
	const char* lpKeyName,
	const int32vec* vec
);

#endif


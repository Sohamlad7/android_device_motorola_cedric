/******************************************************************************
 *
 * $Id: AKCommon.h 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#ifndef AKMD_INC_AKCOMMON_H
#define AKMD_INC_AKCOMMON_H

#include <errno.h>     //errno
#include <stdarg.h>    //va_list
#include <stdio.h>     //frpintf
#include <stdlib.h>    //atoi
#include <string.h>    //memset
#include <unistd.h>

#include "AKMLog.h"

/*** Constant definition ******************************************************/

/* Definition for operation mode */
#define OPMODE_CONSOLE      (0x01)

/*** Type declaration *********************************************************/

/*** Global variables *********************************************************/
extern int g_stopRequest;	/*!< 0:Not stop,  1:Stop */
extern int g_opmode;		/*!< 0:Daemon mode, 1:Console mode. */
extern int g_dbgzone;		/*!< Debug zone. */

/*** Prototype of function ****************************************************/

#endif //AKMD_INC_AKCOMMON_H


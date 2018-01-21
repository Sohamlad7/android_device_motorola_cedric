/******************************************************************************
 *
 * $Id: AKMLog.h 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#ifndef AKMD_INC_AKMLOG_H
#define AKMD_INC_AKMLOG_H

#include <cutils/log.h>

/*** Constant definition ******************************************************/
#undef LOG_TAG
#define LOG_TAG "AKMD2"

#ifndef ALOGE
#ifdef LOGE
#define ALOGE	LOGE
#endif
#endif
#ifndef ALOGE_IF
#ifdef LOGE_IF
#define ALOGE_IF	LOGE_IF
#endif
#endif

#ifndef ALOGW
#ifdef LOGW
#define ALOGW	LOGW
#endif
#endif
#ifndef ALOGW_IF
#ifdef LOGW_IF
#define ALOGW_IF	LOGW_IF
#endif
#endif

#ifndef ALOGI
#ifdef LOGI
#define ALOGI	LOGI
#endif
#endif
#ifndef ALOGI_IF
#ifdef LOGI_IF
#define ALOGI_IF	LOGI_IF
#endif
#endif

#ifndef ALOGD
#ifdef LOGD
#define ALOGD	LOGD
#endif
#endif
#ifndef ALOGD_IF
#ifdef LOGD_IF
#define ALOGD_IF	LOGD_IF
#endif
#endif

#ifndef ALOGV
#ifdef LOGV
#define ALOGV	LOGV
#endif
#endif
#ifndef ALOGV_IF
#ifdef LOGV_IF
#define ALOGV_IF	LOGV_IF
#endif
#endif


#define DATA_AREA01	0x0001
#define DATA_AREA02	0x0002
#define DATA_AREA03	0x0004
#define DATA_AREA04	0x0008
#define DATA_AREA05	0x0010
#define DATA_AREA06	0x0020
#define DATA_AREA07	0x0040
#define DATA_AREA08	0x0080
#define DATA_AREA09	0x0100
#define DATA_AREA10	0x0200
#define DATA_AREA11	0x0400
#define DATA_AREA12	0x0800
#define DATA_AREA13	0x1000
#define DATA_AREA14	0x2000
#define DATA_AREA15	0x4000
#define DATA_AREA16	0x8000


/* Debug area definition */
#define AKMDBG_DEBUG		DATA_AREA01 /*<! Debug output */
#define AKMDBG_DUMP			DATA_AREA02 /*<! Error Dump */
#define AKMDBG_EXECTIME		DATA_AREA03	/*<! Execution flags */
#define AKMDBG_GETINTERVAL	DATA_AREA04	/*<! Interval */
#define AKMDBG_VECTOR		DATA_AREA05	/*<! Sensor Vector */
#define AKMDBG_D6D			DATA_AREA06	/*<! Direction6D */
#define AKMDBG_MAGDRV		DATA_AREA07	/*<! AKM driver's data */
#define AKMDBG_ACCDRV		DATA_AREA08	/*<! Acceleration driver's data */
#define AKMDBG_GYRDRV		DATA_AREA09	/*<! Gyroscope driver's data */
#define AKMDBG_DISP1		DATA_AREA10	/*<! DispMessage level1 */
#define AKMDBG_DISP2		DATA_AREA11	/*<! DispMessage level2 */

#ifndef ENABLE_AKMDEBUG
#define ENABLE_AKMDEBUG		(0)	/* Eanble debug output when it is 1. */
#endif

/***** Debug output ******************************************/
#if ENABLE_AKMDEBUG
#define AKMDEBUG(flag, format, ...) ALOGD((format), ##__VA_ARGS__)
#else
#define AKMDEBUG(flag, format, ...)
#endif

/***** Error output *******************************************/
#define AKMERROR \
	  (ALOGE("%s:%d Error.", __FUNCTION__, __LINE__))

#define AKMERROR_STR(api) \
	  (ALOGE("%s:%d %s Error (%s).", \
	  		  __FUNCTION__, __LINE__, (api), strerror(errno)))

/*** Type declaration *********************************************************/

/*** Global variables *********************************************************/

/*** Prototype of function ****************************************************/

#endif //AKMD_INC_AKMLOG_H


/******************************************************************************
 *
 * $Id: AKMD_Driver.h 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#ifndef AKMD_INC_AKMD_DRIVER_H
#define AKMD_INC_AKMD_DRIVER_H

/* Device driver */
#include "linux/stml0xx_akm.h"

#include <stdint.h>			/* int8_t, int16_t etc. */

/*** Constant definition ******************************************************/
#define AKD_TRUE	1		/*!< Represents true */
#define AKD_FALSE	0		/*!< Represents false */
#define AKD_SUCCESS	1		/*!< Represents success.*/
#define AKD_FAIL	0		/*!< Represents fail. */
#define AKD_ERROR	-1		/*!< Represents error. */

#define AKD_ENABLE	1
#define AKD_DISABLE	0

/*! 0:Don't Output data, 1:Output data */
#define AKD_DBG_DATA	0
/*! Typical interval in ns */
#define AKM_MEASUREMENT_TIME_NS	((AKM_MEASURE_TIME_US) * 1000)


/*** Type declaration *********************************************************/
typedef unsigned char BYTE;

/*!
 Open device driver.
 This function opens device driver of other sensor.
 @return If this function succeeds, the return value is #AKD_SUCCESS. Otherwise
 the return value is #AKD_FAIL.
 */
typedef int16_t(*ACCFNC_INITDEVICE)(void);

/*!
 Close device driver.
 This function closes device drivers of acceleration sensor.
 */
typedef void(*ACCFNC_DEINITDEVICE)(void);

/*!
 Enable or disable sensor
*/
typedef int16_t(*ACCFNC_SET_ENABLE)(const int8_t enabled);

/*!
 Set delay value
*/
typedef int16_t(*ACCFNC_SET_DELAY)(const int64_t ns);

/*!
 Acquire data from other sensor.
 @return If this function succeeds, the return value is #AKD_SUCCESS. Otherwise
 the return value is #AKD_FAIL.
 @param[out] data A data array. The coordinate system and the unit 
 follows the sensor local definition.
 */
typedef int16_t(*ACCFNC_GETACCDATA)(int16_t data[3]);

/*!
 Acquire offset from other sensor.
 @return If this function succeeds, the return value is #AKD_SUCCESS. Otherwise
 the return value is #AKD_FAIL.
 @param[out] offset A offset data array. The coordinate system and the unit 
 follows the sensor local definition.
 */
typedef int16_t(*ACCFNC_GETACCOFFSET)(int16_t offset[3]);

/*!
 Convert from sensor native format to AKSC format
 @param[out] vec A data array. The coordinate system of the
 acquired data follows the definition of Android. Unit is SmartCompass.
*/
typedef void(*ACCFNC_GETACCVEC)(
		const int16_t data[3],
		const int16_t offset[3],
		int16_t vec[3]);

/*** Global variables *********************************************************/

/*** Prototype of Function  ***************************************************/

int16_t AKD_InitDevice(void);

void AKD_DeinitDevice(void);

int16_t AKD_TxData(
		const BYTE address,
		const BYTE* data,
		const uint16_t numberOfBytesToWrite);

int16_t AKD_RxData(
		const BYTE address,
		BYTE* data,
		const uint16_t numberOfBytesToRead);

int16_t AKD_Reset(void);

int16_t AKD_GetSensorInfo(BYTE data[AKM_SENSOR_INFO_SIZE]);

int16_t AKD_GetSensorConf(BYTE data[AKM_SENSOR_CONF_SIZE]);

int16_t AKD_GetMagneticData(BYTE data[AKM_SENSOR_DATA_SIZE]);

void AKD_SetYPR(const int buf[AKM_YPR_DATA_SIZE]);

int16_t AKD_GetOpenStatus(int* status);

int16_t AKD_GetCloseStatus(int* status);

int16_t AKD_SetMode(const BYTE mode);

int16_t AKD_GetDelay(int64_t delay[AKM_NUM_SENSORS]);

int16_t AKD_GetLayout(int16_t* layout);

int16_t AKD_AccSetEnable(int8_t enabled);

int16_t AKD_AccSetDelay(int64_t delay);

int16_t AKD_GetAccelerationData(int16_t data[3]);

int16_t AKD_GetAccelerationOffset(int16_t offset[3]);

void AKD_GetAccelerationVector(
		const int16_t data[3],
		const int16_t offset[3],
		int16_t vec[3]);

int16_t AKD_WaitReady(void);

#endif //AKMD_INC_AKMD_DRIVER_H


/******************************************************************************
 *
 * $Id: Acc_aot.c 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#include <fcntl.h>
#include "Acc_aot.h"
#include "AKCommon.h"	/* For AKMERROR */
#include "Sensors.h"

extern int s_fdDev;

/* Initialize communication device. See "AKMD_Driver.h" */
int16_t AOT_InitDevice(void)
{
	/* When this function is called, the device file is already opened. */
	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}

	return AKD_SUCCESS;
}

/* Release communication device and resources. See "AKMD_Driver.h" */
void AOT_DeinitDevice(void)
{
	/* When this function is called, the device file is already closed. */
}

int16_t AOT_SetEnable(const int8_t enabled)
{
	/* AOT cannot control device */
	return AKD_SUCCESS;
}

int16_t AOT_SetDelay(const int64_t ns)
{
	/* AOT cannot control device */
	return AKD_SUCCESS;
}

int16_t AOT_GetAccData(int16_t data[3])
{
	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}

	/* Get acceleration data from "/dev/Acc_aot" */
	if (ioctl(s_fdDev, ECS_IOCTL_GET_ACCEL, data) < 0) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	}

	/* Convert scale */
	data[0] *= (AKSC_LSG / LSG);
	data[1] *= (AKSC_LSG / LSG);
	data[2] *= (AKSC_LSG / LSG);

	AKMDEBUG(AKMDBG_ACCDRV, "%s: acc=%d, %d, %d\n",
			__FUNCTION__, data[0], data[1], data[2]);

	return AKD_SUCCESS;
}

int16_t AOT_GetAccOffset(int16_t offset[3])
{
	offset[0] = 0;
	offset[1] = 0;
	offset[2] = 0;
	return AKD_SUCCESS;
}

void AOT_GetAccVector(const int16_t data[3], const int16_t offset[3], int16_t vec[3])
{
	vec[0] = (int16_t)(data[0] - offset[0]);
	vec[1] = (int16_t)(data[1] - offset[1]);
	vec[2] = (int16_t)(data[2] - offset[2]);
}


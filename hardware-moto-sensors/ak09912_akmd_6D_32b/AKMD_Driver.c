/******************************************************************************
 *
 * $Id: AKMD_Driver.c 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#include "AKCommon.h"		// DBGPRINT()
#include "AKMD_Driver.h"

#define AKM_MEASURE_RETRY_NUM	3
int s_fdDev = -1;

/* Include proper acceleration file. */
#ifdef AKMD_ACC_EXTERNAL
#include "Acc_aot.h"
static ACCFNC_INITDEVICE Acc_InitDevice		= AOT_InitDevice;
static ACCFNC_DEINITDEVICE Acc_DeinitDevice = AOT_DeinitDevice;
static ACCFNC_SET_ENABLE Acc_SetEnable		= AOT_SetEnable;
static ACCFNC_SET_DELAY Acc_SetDelay		= AOT_SetDelay;
static ACCFNC_GETACCDATA Acc_GetAccData		= AOT_GetAccData;
static ACCFNC_GETACCOFFSET Acc_GetAccOffset	= AOT_GetAccOffset;
static ACCFNC_GETACCVEC Acc_GetAccVector	= AOT_GetAccVector;

#else
#ifdef AKMD_ACC_ADXL346
#include "Acc_adxl34x.h"
static ACCFNC_INITDEVICE Acc_InitDevice		= ADXL_InitDevice;
static ACCFNC_DEINITDEVICE Acc_DeinitDevice = ADXL_DeinitDevice;
static ACCFNC_SET_ENABLE Acc_SetEnable		= ADXL_SetEnable;
static ACCFNC_SET_DELAY Acc_SetDelay		= ADXL_SetDelay;
static ACCFNC_GETACCDATA Acc_GetAccData		= ADXL_GetAccData;
static ACCFNC_GETACCOFFSET Acc_GetAccOffset	= ADXL_GetAccOffset;
static ACCFNC_GETACCVEC Acc_GetAccVector	= ADXL_GetAccVector;
#endif
#ifdef AKMD_ACC_KXTF9
#include "Acc_kxtf9.h"
static ACCFNC_INITDEVICE Acc_InitDevice		= KXTF9_InitDevice;
static ACCFNC_DEINITDEVICE Acc_DeinitDevice = KXTF9_DeinitDevice;
static ACCFNC_SET_ENABLE Acc_SetEnable		= KXTF9_SetEnable;
static ACCFNC_SET_DELAY Acc_SetDelay		= KXTF9_SetDelay;
static ACCFNC_GETACCDATA Acc_GetAccData		= KXTF9_GetAccData;
static ACCFNC_GETACCOFFSET Acc_GetAccOffset	= KXTF9_GetAccOffset;
static ACCFNC_GETACCVEC Acc_GetAccVector	= KXTF9_GetAccVector;
#endif
#ifdef AKMD_ACC_DUMMY
#include "Acc_dummy.h"
static ACCFNC_INITDEVICE Acc_InitDevice		= ACC_DUMMY_InitDevice;
static ACCFNC_DEINITDEVICE Acc_DeinitDevice	= ACC_DUMMY_DeinitDevice;
static ACCFNC_SET_ENABLE Acc_SetEnable		= ACC_DUMMY_SetEnable;
static ACCFNC_SET_DELAY  Acc_SetDelay		= ACC_DUMMY_SetDelay;
static ACCFNC_GETACCDATA Acc_GetAccData		= ACC_DUMMY_GetAccData;
static ACCFNC_GETACCOFFSET Acc_GetAccOffset	= ACC_DUMMY_GetAccOffset;
static ACCFNC_GETACCVEC Acc_GetAccVector	= ACC_DUMMY_GetAccVector;
#endif
#endif

/*!
 Open device driver.
 This function opens both device drivers of magnetic sensor and acceleration
 sensor. Additionally, some initial hardware settings are done, such as
 measurement range, built-in filter function and etc.
 @return If this function succeeds, the return value is #AKD_SUCCESS.
 Otherwise the return value is #AKD_FAIL.
 */
int16_t AKD_InitDevice(void)
{
	if (s_fdDev < 0) {
		// Open magnetic sensor's device driver.
		if ((s_fdDev = open("/dev/" AKM_MISCDEV_NAME, O_RDWR)) < 0) {
			AKMERROR_STR("open");
			goto INIT_FAIL;
		}
	}

	if (Acc_InitDevice() != AKD_SUCCESS) {
		goto INIT_FAIL;
	}

	return AKD_SUCCESS;

INIT_FAIL:
	AKD_DeinitDevice();
	return AKD_FAIL;
}

/*!
 Close device driver.
 This function closes both device drivers of magnetic sensor and acceleration
 sensor.
 */
void AKD_DeinitDevice(void)
{
	if (s_fdDev >= 0) {
		close(s_fdDev);
		s_fdDev = -1;
	}

	Acc_DeinitDevice();
}

/*!
 Writes data to a register of the AKM E-Compass.  When more than one byte of
 data is specified, the data is written in contiguous locations starting at an
 address specified in \a address.
 @return If this function succeeds, the return value is #AKD_SUCCESS. Otherwise
 the return value is #AKD_FAIL.
 @param[in] address Specify the address of a register in which data is to be
 written.
 @param[in] data Specify data to write or a pointer to a data array containing
 the data.  When specifying more than one byte of data, specify the starting
 address of the array.
 @param[in] numberOfBytesToWrite Specify the number of bytes that make up the
 data to write.  When a pointer to an array is specified in data, this argument
 equals the number of elements of the array.
 */
int16_t AKD_TxData(
		const BYTE address,
		const BYTE * data,
		const uint16_t numberOfBytesToWrite)
{
	int i;
	char buf[AKM_RWBUF_SIZE];

	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}
	if (numberOfBytesToWrite > (AKM_RWBUF_SIZE-2)) {
		AKMERROR;
		return AKD_FAIL;
	}

	buf[0] = numberOfBytesToWrite + 1;
	buf[1] = address;

	for (i = 0; i < numberOfBytesToWrite; i++) {
		buf[i + 2] = data[i];
	}
	if (ioctl(s_fdDev, ECS_IOCTL_WRITE, buf) < 0) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	} else {

#if ENABLE_AKMDEBUG
		AKMDEBUG(AKMDBG_MAGDRV, "addr(HEX)=%02x data(HEX)=", address);
		for (i = 0; i < numberOfBytesToWrite; i++) {
			AKMDEBUG(AKMDBG_MAGDRV, " %02x", data[i]);
		}
		AKMDEBUG(AKMDBG_MAGDRV, "\n");
#endif
		return AKD_SUCCESS;
	}
}

/*!
 Acquires data from a register or the EEPROM of the AKM E-Compass.
 @return If this function succeeds, the return value is #AKD_SUCCESS. Otherwise
 the return value is #AKD_FAIL.
 @param[in] address Specify the address of a register from which data is to be
 read.
 @param[out] data Specify a pointer to a data array which the read data are
 stored.
 @param[in] numberOfBytesToRead Specify the number of bytes that make up the
 data to read.  When a pointer to an array is specified in data, this argument
 equals the number of elements of the array.
 */
int16_t AKD_RxData(
		const BYTE address,
		BYTE * data,
		const uint16_t numberOfBytesToRead)
{
	int i;
	char buf[AKM_RWBUF_SIZE];

	memset(data, 0, numberOfBytesToRead);

	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}
	if (numberOfBytesToRead > (AKM_RWBUF_SIZE-1)) {
		AKMERROR;
		return AKD_FAIL;
	}

	buf[0] = numberOfBytesToRead;
	buf[1] = address;

	if (ioctl(s_fdDev, ECS_IOCTL_READ, buf) < 0) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	} else {
		for (i = 0; i < numberOfBytesToRead; i++) {
			data[i] = buf[i + 1];
		}
#if ENABLE_AKMDEBUG
		AKMDEBUG(AKMDBG_MAGDRV, "addr(HEX)=%02x len=%d data(HEX)=",
				address, numberOfBytesToRead);
		for (i = 0; i < numberOfBytesToRead; i++) {
			AKMDEBUG(AKMDBG_MAGDRV, " %02x", data[i]);
		}
		AKMDEBUG(AKMDBG_MAGDRV, "\n");
#endif
		return AKD_SUCCESS;
	}
}

/*!
 Reset the e-compass.
 @return If this function succeeds, the return value is #AKD_SUCCESS. Otherwise
 the return value is #AKD_FAIL.
 */
int16_t AKD_Reset(void) {
	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}
	if (ioctl(s_fdDev, ECS_IOCTL_RESET, NULL) < 0) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	}
	return AKD_SUCCESS;
}

/*!
 Get magnetic sensor information from device. This function returns WIA value.
 @return If this function succeeds, the return value is #AKD_SUCCESS. Otherwise
 the return value is #AKD_FAIL.
 @param[out] data An information data array. The size should be larger than
 #AKM_SENSOR_INFO_SIZE
 */
int16_t AKD_GetSensorInfo(BYTE data[AKM_SENSOR_INFO_SIZE])
{
	memset(data, 0, AKM_SENSOR_INFO_SIZE);

	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}
	if (ioctl(s_fdDev, ECS_IOCTL_GET_INFO, data) < 0) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	}
	return AKD_SUCCESS;
}

/*!
 Get magnetic sensor configuration from device. This function returns ASA value.
 @return If this function succeeds, the return value is #AKD_SUCCESS. Otherwise
 the return value is #AKD_FAIL.
 @param[out] data An configuration data array. The size should be larger than
 #AKM_SENSOR_CONF_SIZE
 */
int16_t AKD_GetSensorConf(BYTE data[AKM_SENSOR_CONF_SIZE])
{
	memset(data, 0, AKM_SENSOR_CONF_SIZE);

	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}
	if (ioctl(s_fdDev, ECS_IOCTL_GET_CONF, data) < 0) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	}
	return AKD_SUCCESS;
}


/*!
 Acquire magnetic data from AKM E-Compass. If measurement is not done, this
 function waits until measurement completion.
 @return If this function succeeds, the return value is #AKD_SUCCESS. Otherwise
 the return value is #AKD_FAIL.
 @param[out] data A magnetic data array. The size should be larger than
 #AKM_SENSOR_DATA_SIZE.
 */
int16_t AKD_GetMagneticData(BYTE data[AKM_SENSOR_DATA_SIZE + AKM_SENSOR_TIME_SIZE])
{
	int ret;
	int i;

	memset(data, 0, AKM_SENSOR_DATA_SIZE + AKM_SENSOR_TIME_SIZE);

	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}

	do {
		ret = ioctl(s_fdDev, ECS_IOCTL_GET_DATA, data);
	} while (ret == -EINTR);

	if (ret < 0)
		return AKD_FAIL;
#if 0
	for (i = 0; i < AKM_MEASURE_RETRY_NUM; i++) {
		ret = ioctl(s_fdDev, ECS_IOCTL_GET_DATA, data);

		if (ret >= 0) {
			/* Success */
			break;
		}
		if (errno != EAGAIN) {
			AKMERROR_STR("ioctl");
			return AKD_FAIL;
		}
		AKMDEBUG(AKMDBG_MAGDRV, "Try Again.");
		usleep(AKM_MEASURE_TIME_US);
	}

	if (i >= AKM_MEASURE_RETRY_NUM) {
		AKMERROR;
		return AKD_FAIL;
	}
#endif

	AKMDEBUG(AKMDBG_MAGDRV,
			"bdata(HEX)= %02x %02x %02x %02x %02x %02x %02x %02x %02x | %02x %02x %02x %02x %02x %02x %02x %02x\n",
			data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8],
            data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16]);

	return AKD_SUCCESS;
}

/*!
 Set calculated data to device driver.
 @param[in] buf
 */
void AKD_SetYPR(const int buf[AKM_YPR_DATA_SIZE + (AKM_SENSOR_TIME_SIZE / sizeof(int))])
{
	if (s_fdDev < 0) {
		AKMERROR;
		return;
	}
	if (ioctl(s_fdDev, ECS_IOCTL_SET_YPR, buf) < 0) {
		AKMERROR_STR("ioctl");
	}
}

/*!
 */
int16_t AKD_GetOpenStatus(int* status)
{
	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}
	if (ioctl(s_fdDev, ECS_IOCTL_GET_OPEN_STATUS, status) < 0) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	}
	return AKD_SUCCESS;
}

/*!
 */
int16_t AKD_GetCloseStatus(int* status)
{
	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}
	if (ioctl(s_fdDev, ECS_IOCTL_GET_CLOSE_STATUS, status) < 0) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	}
	return AKD_SUCCESS;
}

/*!
 Set AKM E-Compass to the specific mode.
 @return If this function succeeds, the return value is #AKD_SUCCESS. Otherwise
 the return value is #AKD_FAIL.
 @param[in] mode This value should be one of the AKM_MODE which is defined in
 header file.
 */
int16_t AKD_SetMode(const BYTE mode)
{
	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}
	if (ioctl(s_fdDev, ECS_IOCTL_SET_MODE, & mode) < 0) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	}
	return AKD_SUCCESS;
}

/*!
 Acquire delay
 @return If this function succeeds, the return value is #AKD_SUCCESS. Otherwise
 the return value is #AKD_FAIL.
 @param[out] delay A delay in microsecond.
 */
int16_t AKD_GetDelay(int64_t delay[AKM_NUM_SENSORS])
{
	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}
	if (ioctl(s_fdDev, ECS_IOCTL_GET_DELAY, delay) < 0) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	}
	return AKD_SUCCESS;
}

/*!
 Get layout information from device driver, i.e. platform data.
 */
int16_t AKD_GetLayout(int16_t* layout)
{
	char tmp;

	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}
	if (ioctl(s_fdDev, ECS_IOCTL_GET_LAYOUT, &tmp) < 0) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	}

	*layout = tmp;
	return AKD_SUCCESS;
}

/*!  */
int16_t AKD_AccSetEnable(int8_t enabled)
{
	return Acc_SetEnable(enabled);
}

/*!  */
int16_t AKD_AccSetDelay(int64_t delay)
{
	return Acc_SetDelay(delay);
}

/*!  */
int16_t AKD_GetAccelerationData(int16_t data[3])
{
	return Acc_GetAccData(data);
}

/*!  */
int16_t AKD_GetAccelerationOffset(int16_t offset[3])
{
	return Acc_GetAccOffset(offset);
}

/*!  */
void AKD_GetAccelerationVector(
	const int16_t data[3],
	const int16_t offset[3],
	int16_t vec[3])
{
	Acc_GetAccVector(data, offset, vec);
}

int16_t AKD_WaitReady(void)
{
	int retries = 10;
	char boot_status;
	int err;

	if (s_fdDev < 0) {
		AKMERROR;
		return AKD_FAIL;
	}
	while (((err = ioctl(s_fdDev, ECS_IOCTL_CHECK_READY, &boot_status)) == -EAGAIN) && retries) {
		sleep(1);
		retries--;
	}

	if ((retries == 0) || (err < 0) || (boot_status != 1)) {
		AKMERROR_STR("ioctl");
		return AKD_FAIL;
	}

	return AKD_SUCCESS;
}

/******************************************************************************
 *
 * $Id: misc.c 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#include "misc.h"
#include "AKCommon.h"
#include <fcntl.h>
#include <linux/input.h>
#include <time.h> /* ns_to_timespec() */

static int s_fdForm = -1; /*!< FD to formation detect device */

struct AKMD_INTERVAL {
	long interval; /*!< Measurement interval, 32-bit is enough for HDOE */
	int16 decimator; /*!< HDOE decimator */
};

static struct AKMD_INTERVAL s_interval[] = {
	 { 10000000, 10 }, /* 100Hz SENSOR_DELAY_FASTEST */
	 { 16666667,  6 }, /*  60Hz */
	 { 20000000,  5 }, /*  50Hz SENSOR_DELAY_GAME */
	 { 25000000,  4 }, /*  40Hz */
	 { 40000000,  3 }, /*  25Hz */
	 { 50000000,  2 }, /*  20Hz */
	 { 66667000,  2 }, /*  15Hz SENSOR_DELAY_UI */
	{ 100000000,  1 }, /*  10Hz */
	{ 125000000,  1 }, /*   8Hz */
	{ 200000000,  1 }, /*   5Hz SENSOR_DELAY_NORMAL */
	{1000000000,  1 }  /*   1Hz */
};

/*!
 Sleep function in millisecond.
 @param[in] msec Suspend time in millisecond.
 */
inline void msleep(signed int msec)
{
	usleep(1000 * msec);
}

/*!
 Open device driver which detects current formation. This is just a stab of
 interface. Therefore, this function do nothing.
 @return If function succeeds, the return value is 1. Otherwise, the return
 value is 0.
 */
int16 misc_openForm(void)
{
	if (s_fdForm < 0) {
		s_fdForm = 0;
	}
	return 1;
}

/*!
 Close device driver
 */
void misc_closeForm(void)
{
	if (s_fdForm != -1) {
		s_fdForm = -1;
	}
}

/*!
 Get current formation  This is just a stab of interface. Therefore, this
 function do nothing.
 @return The number represents current form.
 */
int16 misc_checkForm(void)
{
	return 0;
}

/*!
 Convert from int64_t value to timespec structure
 @return timespec value
 @param[in] val int64_t value
 */
struct timespec int64_to_timespec(int64_t val)
{
	struct timespec ret;
	ret.tv_sec = (long) (val / 1000000000);
	ret.tv_nsec = val - ret.tv_sec;

	return ret;
}

/*!
 Convert from timespec structure to int64_t value
 @return int64_t value
 @param[in] val timespec value
 */
int64_t timespec_to_int64(struct timespec* val)
{
	return ((int64_t)(val->tv_sec * 1000000000) + (int64_t)val->tv_nsec);
}

/*!
 Calculate the difference between two timespec structure.
 \a begin should be later than \a end.
 @return the difference in int64_t type value.
 @param[in] begin
 @param[in] end
 */
int64_t CalcDuration(struct timespec* begin, struct timespec* end)
{
	struct timespec diff;

	diff.tv_sec = begin->tv_sec - end->tv_sec;

	if ((diff.tv_nsec = begin->tv_nsec - end->tv_nsec) < 0) {
		diff.tv_nsec += 1000000000;
		diff.tv_sec -= 1;
	}

	if (diff.tv_sec < 0) {
		return 0; /* Minimum nanosec */
	}

	return timespec_to_int64(&diff);
}

/*!
 Search and open an input event file by name. This function search the
 directory "/dev/input/".
 @return When this function succeeds, the return value is a file descriptor
 to the found event file. If fails, the return value is -1.
 @param[in] name The name of input device to be searched.
 */
int openInputDevice(const char* name)
{
	const int NUM_EVENT = 16;
	const int BUFSIZE = 32;
	const int NAMESIZE = 128;
	char buf[BUFSIZE];
	char evName[NAMESIZE];
	int i;
	int fd;
	for (i = 0; i < NUM_EVENT; i++) {
		if (snprintf(buf, BUFSIZE, "/dev/input/event%d", i) < 0) {
			break;
		}
		fd = open(buf, O_RDONLY | O_NONBLOCK);
		if (fd >= 0) {
			if (ioctl(fd, EVIOCGNAME(NAMESIZE - 1), &evName) >= 1) {
				AKMDEBUG(AKMDBG_DEBUG, "event%d(%d): %s\n", i, fd, evName);
				if (!strncmp(name, evName, strlen(name))) {
					break;
				}
			}
			close(fd);
		}
		fd = -1;
	}

	AKMDEBUG(AKMDBG_DEBUG, "openInput(%s): fd=%d\n", name, fd);
	return fd;
}

/*!
 Get valid measurement interval and HDOE decimator.
 @return If this function succeeds, the return value is 1. Otherwise 0.
 @param[in,out] time Input a requirement of sensor measurement interval.
 The closest interval will be returned as output.
 @param[out] hdoe_interval When the output interval value is set to looper,
 this decimation value should be used to decimate the HDOE.
 */
int16 GetHDOEDecimator(int64_t* time, int16* hdoe_interval)
{
	const int n = (sizeof(s_interval) / sizeof(s_interval[0]));
	int i;
	int64_t org;

	org = *time;

	for (i = 0; i < n; i++) {
		*time = s_interval[i].interval;
		*hdoe_interval = s_interval[i].decimator;
		if (org <= *time) {
			break;
		}
	}

	AKMDEBUG(AKMDBG_DEBUG, "HDOEDecimator:%lld,%lld\n", org, *time);
	return 1;
}

/*!
 This function convert coordinate system.
 @retval 1 Conversion succeeds.
 @retval 0 Conversion failed.
 @param[in] pat Conversion pattern number.
 @param[in,out] Original vector as input, converted data as output.
*/
int16 ConvertCoordinate(
	const	AKMD_PATNO	pat,
			int16vec*	vec)
{
	int16	tmp;
	switch(pat){
	// Obverse
	case PAT1:
		// This is Android default
		break;
	case PAT2:
		tmp = vec->u.x;
		vec->u.x = vec->u.y;
		vec->u.y = -tmp;
		break;
	case PAT3:
		vec->u.x = -(vec->u.x);
		vec->u.y = -(vec->u.y);
		break;
	case PAT4:
		tmp = vec->u.x;
		vec->u.x = -(vec->u.y);
		vec->u.y = tmp;
		break;
	// Reverse
	case PAT5:
		vec->u.x = -(vec->u.x);
		vec->u.z = -(vec->u.z);
		break;
	case PAT6:
		tmp = vec->u.x;
		vec->u.x = vec->u.y;
		vec->u.y = tmp;
		vec->u.z = -(vec->u.z);
		break;
	case PAT7:
		vec->u.y = -(vec->u.y);
		vec->u.z = -(vec->u.z);
		break;
	case PAT8:
		tmp = vec->u.x;
		vec->u.x = -(vec->u.y);
		vec->u.y = -tmp;
		vec->u.z = -(vec->u.z);
		break;
	default:
		return 0;
	}
	return 1;
}



/******************************************************************************
 *
 * $Id: main.c 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#include <sched.h>
#include <pthread.h>
#include <linux/input.h>

#include "AKCommon.h"
#include "AKMD_Driver.h"
#include "DispMessage.h"
#include "FileIO.h"
#include "FST.h"
#include "Measure.h"
#include "misc.h"

#define ERROR_OPTPARSE			(-1)
#define ERROR_INITDEVICE		(-2)
#define ERROR_HLAYOUT			(-3)
#define ERROR_FUSEROM			(-4)
#define ERROR_GETOPEN_STAT		(-5)
#define ERROR_STARTCLONE		(-6)
#define ERROR_GETCLOSE_STAT		(-7)

/* Global variable. See AKCommon.h file. */
int g_stopRequest = 0;
int g_opmode = 0;
int g_dbgzone = 0;
int g_mainQuit = AKD_FALSE;

/* Static variable. */
static pthread_t s_thread;	/*!< Thread handle */
static FORM_CLASS s_formClass = {
	.open  = misc_openForm,
	.close = misc_closeForm,
	.check = misc_checkForm,
};

/*!
 A thread function which is raised when measurement is started.
 @param[in] args A pointer to #AKSCPRMS structure.
 */
static void* thread_main(void* args)
{
	MeasureSNGLoop((AKSCPRMS *)args);
	return ((void*)0);
}

static void signal_handler(int sig)
{
	if (sig == SIGINT) {
		AKMERROR;
		g_stopRequest = 1;
		g_mainQuit = AKD_TRUE;
	}
}

/*!
 Starts new thread.
 @return If this function succeeds, the return value is 1. Otherwise,
 the return value is 0.
 @param[in] prms A pointer to #AKSCPRMS structure. This pointer is passed
 to the new thread argument.
 */
static int startClone(AKSCPRMS* prms)
{
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	g_stopRequest = 0;
	if (pthread_create(&s_thread, &attr, thread_main, prms) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*!
 Output measurement result. If this application is run as ConsoleMode,
 the measurement result is output to console. If this application is run as
 DaemonMode, the measurement result is written to device driver.
 @param[in] prms pointer to #AKSCPRMS structure.
 @param[in] flag This flag shows which data contains the valid data.
 The device driver will report only the valid data to HAL layer.
 */
void Disp_MeasurementResultHook(AKSCPRMS* prms, const uint16 flag, uint8 *time)
{
	int rbuf[AKM_YPR_DATA_SIZE + (AKM_SENSOR_TIME_SIZE / sizeof(int))] = { 0 };
	int16vec rawmag;

	/* Coordinate system is already converted to Android */
	rbuf[0] = flag;				/* Data flag */
	rbuf[1] = prms->m_avec.u.x;	/* Ax */
	rbuf[2] = prms->m_avec.u.y;	/* Ay */
	rbuf[3] = prms->m_avec.u.z;	/* Az */
	rbuf[4] = 3;				/* Acc status */
	rbuf[5] = prms->m_hvec.u.x;	/* Mx */
	rbuf[6] = prms->m_hvec.u.y;	/* My */
	rbuf[7] = prms->m_hvec.u.z;	/* Mz */
	rbuf[8] = prms->m_hdst;		/* Mag status */
	/* Orientation (Q6 format)*/
	rbuf[9] = prms->m_theta;	/* yaw	(deprecate) */
	rbuf[10] = prms->m_phi180;	/* pitch (deprecate) */
	rbuf[11] = prms->m_eta90;	/* roll  (deprecate) */
	/* Axis conversion, from AKSC to Android is done here */
	/* RotVec (AKSC Q4 format deg/sec ) */
	rbuf[12] = prms->m_quat.u.y;
	rbuf[13] = prms->m_quat.u.x * (-1);
	rbuf[14] = prms->m_quat.u.z * (-1);
	rbuf[15] = prms->m_quat.u.w;

	/* Get the uncalibrated reading */
	rawmag.u.x = prms->m_hdata[0].u.x;
	rawmag.u.y = prms->m_hdata[0].u.y;
	rawmag.u.z = prms->m_hdata[0].u.z;
	ConvertCoordinate(prms->m_hlayout, &rawmag);
	rbuf[16] = rawmag.u.x;
	rbuf[17] = rawmag.u.y;
	rbuf[18] = rawmag.u.z;

	memcpy(&rbuf[AKM_YPR_DATA_SIZE], time, AKM_SENSOR_TIME_SIZE);

	AKD_SetYPR(rbuf);

	if (g_opmode & OPMODE_CONSOLE) {
		Disp_MeasurementResult(prms);
	}
}

/*!
 This function parse the option.
 @retval 1 Parse succeeds.
 @retval 0 Parse failed.
 @param[in] argc Argument count
 @param[in] argv Argument vector
 @param[out] layout_patno
 */
int OptParse(
	int	argc,
	char*	argv[],
	AKMD_PATNO*	hlayout_patno)
{
	int	opt;
	char optVal;

	/* Initial value */
	*hlayout_patno = PAT_INVALID;

	while ((opt = getopt(argc, argv, "sm:z:")) != -1) {
		switch(opt){
			case 'm':
				optVal = (char)(optarg[0] - '0');
				if ((PAT1 <= optVal) && (optVal <= PAT8)) {
					*hlayout_patno = (AKMD_PATNO)optVal;
				}
				break;
			case 'z':
				/* If error detected, hopefully 0 is returned. */
				g_dbgzone = (int)strtol(optarg, (char**)NULL, 0);
				break;
			case 's':
				g_opmode |= OPMODE_CONSOLE;
				break;
			default:
				ALOGE("%s: Invalid argument", argv[0]);
				return 0;
		}
	}

	AKMDEBUG(AKMDBG_DEBUG, "%s: Mode=0x%04X\n", __FUNCTION__, g_opmode);
	AKMDEBUG(AKMDBG_DEBUG, "%s: Layout=%d\n", __FUNCTION__, *hlayout_patno);
	AKMDEBUG(AKMDBG_DEBUG, "%s: Dbg Zone=0x%04X\n", __FUNCTION__, g_dbgzone);

	return 1;
}

/*!
 This is main function.
 */
int main(int argc, char **argv)
{
	AKSCPRMS prms;
	int retValue = 0;

	/* Show the version info of this software. */
	Disp_StartMessage();

#if ENABLE_AKMDEBUG
	/* Register signal handler */
	signal(SIGINT, signal_handler);
#endif

#if ENABLE_FORMATION
	RegisterFormClass(&s_formClass);
#endif

	/* Initialize parameters structure. */
	InitAKSCPRMS(&prms);

	/* Parse command-line options */
	if (OptParse(argc, argv, &prms.m_hlayout) == 0) {
		retValue = ERROR_OPTPARSE;
		goto THE_END_OF_MAIN_FUNCTION;
	}

	/* Open device driver. */
	if (AKD_InitDevice() != AKD_SUCCESS) {
		retValue = ERROR_INITDEVICE;
		goto THE_END_OF_MAIN_FUNCTION;
	}

	/* Make sure the sensor hub has booted */
	if (AKD_WaitReady() != AKD_SUCCESS) {
		retValue = ERROR_INITDEVICE;
		goto THE_END_OF_MAIN_FUNCTION;
	}

	/* If layout is not specified with argument, get parameter from driver */
	if (prms.m_hlayout == PAT_INVALID) {
		int16_t n;
		if (AKD_GetLayout(&n) == AKD_SUCCESS) {
			if ((PAT1 <= n) && (n <= PAT8)) {
				prms.m_hlayout = (AKMD_PATNO)n;
			}
		}
		/* Error */
		if (prms.m_hlayout == PAT_INVALID) {
			ALOGE("Magnetic sensor's layout is not specified.");
			retValue = ERROR_HLAYOUT;
			goto THE_END_OF_MAIN_FUNCTION;
		}
	}

	/* Read Fuse ROM */
	if (ReadFUSEROM(&prms) != AKRET_PROC_SUCCEED) {
		retValue = ERROR_FUSEROM;
		goto THE_END_OF_MAIN_FUNCTION;
	}

	/* PDC */
	LoadPDC(&prms);

	/* Here is the Main Loop */
	if (g_opmode & OPMODE_CONSOLE) {
		/*** Console Mode *********************************************/
		while (AKD_TRUE) {
			/* Select operation */
			switch (Menu_Main()) {
				case MODE_FST:
					FST_Body();
					break;

				case MODE_MeasureSNG:
					/* Read Parameters from file. */
					if (LoadParameters(&prms) == 0) {
						SetDefaultPRMS(&prms);
					}
					/* Reset flag */
					g_stopRequest = 0;
					/* Measurement routine */
					MeasureSNGLoop(&prms);

					/* Write Parameters to file. */
					SaveParameters(&prms);
					break;

				case MODE_OffsetCalibration:
					/* Read Parameters from file. */
					if (LoadParameters(&prms) == 0) {
						SetDefaultPRMS(&prms);
					}
					/* measure offset (NOT sensitivity) */
					if (SimpleCalibration(&prms) == AKRET_PROC_SUCCEED) {
						SaveParameters(&prms);
					}
					break;

				case MODE_Quit:
					goto THE_END_OF_MAIN_FUNCTION;
					break;

				default:
					AKMDEBUG(AKMDBG_DEBUG, "Unknown operation mode.\n");
					break;
			}
		}
	} else {
		/*** Daemon Mode *********************************************/
		while (g_mainQuit == AKD_FALSE) {
			int st = 0;
			/* Wait until device driver is opened. */
			if (AKD_GetOpenStatus(&st) != AKD_SUCCESS) {
				retValue = ERROR_GETOPEN_STAT;
				goto THE_END_OF_MAIN_FUNCTION;
			}
			if (st == 0) {
				AKMDEBUG(AKMDBG_DEBUG, "Suspended.");
			} else {
				AKMDEBUG(AKMDBG_DEBUG, "Compass Opened.");
				/* Read Parameters from file. */
				if (LoadParameters(&prms) == 0) {
					SetDefaultPRMS(&prms);
				}
				/* Reset flag */
				g_stopRequest = 0;
				/* Start measurement thread. */
				if (startClone(&prms) == 0) {
					retValue = ERROR_STARTCLONE;
					goto THE_END_OF_MAIN_FUNCTION;
				}

				/* Wait until device driver is closed. */
				if (AKD_GetCloseStatus(&st) != AKD_SUCCESS) {
					retValue = ERROR_GETCLOSE_STAT;
					g_mainQuit = AKD_TRUE;
				}
				/* Wait thread completion. */
				g_stopRequest = 1;
				pthread_join(s_thread, NULL);
				AKMDEBUG(AKMDBG_DEBUG, "Compass Closed.");

				/* Write Parameters to file. */
				SaveParameters(&prms);
			}
		}
	}

THE_END_OF_MAIN_FUNCTION:

	/* Close device driver. */
	AKD_DeinitDevice();

	/* Show the last message. */
	Disp_EndMessage(retValue);

	return retValue;
}



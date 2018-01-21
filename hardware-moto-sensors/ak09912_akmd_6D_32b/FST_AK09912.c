/******************************************************************************
 *
 * $Id: FST_AK09912.c 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#include "AKCommon.h"
#include "AKMD_Driver.h"
#include "FST.h"
#include "misc.h"

#ifndef AKMD_FOR_AK09912
#error "AKMD parameter is not set"
#endif

#define TLIMIT_NO_RST_WIA1				"1-3"
#define TLIMIT_TN_RST_WIA1				"RST_WIA1"
#define TLIMIT_LO_RST_WIA1				0x48
#define TLIMIT_HI_RST_WIA1				0x48
#define TLIMIT_NO_RST_WIA2				"1-4"
#define TLIMIT_TN_RST_WIA2				"RST_WIA2"
#define TLIMIT_LO_RST_WIA2				0x04
#define TLIMIT_HI_RST_WIA2				0x04

#define TLIMIT_NO_WR_CNTL2				"1-7"
#define TLIMIT_TN_WR_CNTL2				"WR_CNTL2"
#define TLIMIT_LO_WR_CNTL2				0x1F
#define TLIMIT_HI_WR_CNTL2				0x1F
#define TLIMIT_NO_ASAX					"1-8"
#define TLIMIT_TN_ASAX					"ASAX"
#define TLIMIT_LO_ASAX					1
#define TLIMIT_HI_ASAX					254
#define TLIMIT_NO_ASAY					"1-9"
#define TLIMIT_TN_ASAY					"ASAY"
#define TLIMIT_LO_ASAY					1
#define TLIMIT_HI_ASAY					254
#define TLIMIT_NO_ASAZ					"1-10"
#define TLIMIT_TN_ASAZ					"ASAZ"
#define TLIMIT_LO_ASAZ					1
#define TLIMIT_HI_ASAZ					254

#define TLIMIT_NO_SNG_ST1				"2-4"
#define TLIMIT_TN_SNG_ST1				"SNG_ST1"
#define TLIMIT_LO_SNG_ST1				1
#define TLIMIT_HI_SNG_ST1				1

#define TLIMIT_NO_SNG_HX				"2-5"
#define TLIMIT_TN_SNG_HX				"SNG_HX"
#define TLIMIT_LO_SNG_HX				-32751
#define TLIMIT_HI_SNG_HX				32751

#define TLIMIT_NO_SNG_HY				"2-7"
#define TLIMIT_TN_SNG_HY				"SNG_HY"
#define TLIMIT_LO_SNG_HY				-32751
#define TLIMIT_HI_SNG_HY				32751

#define TLIMIT_NO_SNG_HZ				"2-9"
#define TLIMIT_TN_SNG_HZ				"SNG_HZ"
#define TLIMIT_LO_SNG_HZ				-32751
#define TLIMIT_HI_SNG_HZ				32751

#define TLIMIT_NO_SNG_TMPS				"2-11"
#define TLIMIT_TN_SNG_TMPS				"SNG_TMPS"
#define TLIMIT_LO_SNG_TMPS				0x28	// +85 degree
#define TLIMIT_HI_SNG_TMPS				0xE0	// -30 degree

#define TLIMIT_NO_SNG_ST2				"2-12"
#define TLIMIT_TN_SNG_ST2				"SNG_ST2"
#define TLIMIT_LO_SNG_ST2				0
#define TLIMIT_HI_SNG_ST2				0

#define TLIMIT_NO_SLF_ST1				"2-15"
#define TLIMIT_TN_SLF_ST1				"SLF_ST1"
#define TLIMIT_LO_SLF_ST1				1
#define TLIMIT_HI_SLF_ST1				1

#define TLIMIT_NO_SLF_RVHX				"2-16"
#define TLIMIT_TN_SLF_RVHX				"SLF_REVSHX"
#define TLIMIT_LO_SLF_RVHX				-200
#define TLIMIT_HI_SLF_RVHX				200

#define TLIMIT_NO_SLF_RVHY				"2-18"
#define TLIMIT_TN_SLF_RVHY				"SLF_REVSHY"
#define TLIMIT_LO_SLF_RVHY				-200
#define TLIMIT_HI_SLF_RVHY				200

#define TLIMIT_NO_SLF_RVHZ				"2-20"
#define TLIMIT_TN_SLF_RVHZ				"SLF_REVSHZ"
#define TLIMIT_LO_SLF_RVHZ				-1600
#define TLIMIT_HI_SLF_RVHZ				-400

#define TLIMIT_NO_SLF_ST2				"2-22"
#define TLIMIT_TN_SLF_ST2				"SLF_ST2"
#define TLIMIT_LO_SLF_ST2				0
#define TLIMIT_HI_SLF_ST2				0

/*!
 @return If @a testdata is in the range of between @a lolimit and @a hilimit,
 the return value is 1, otherwise -1.
 @param[in] testno   A pointer to a text string.
 @param[in] testname A pointer to a text string.
 @param[in] testdata A data to be tested.
 @param[in] lolimit  The maximum allowable value of @a testdata.
 @param[in] hilimit  The minimum allowable value of @a testdata.
 @param[in,out] pf_total
 */
int16
TEST_DATA(const char testno[],
		  const char testname[],
          const int16 testdata,
		  const int16 lolimit,
		  const int16 hilimit,
          int16 * pf_total)
{
	int16 pf;                     //Pass;1, Fail;-1

	if ((testno == NULL) && (strncmp(testname, "START", 5) == 0)) {
		// Display header
		AKMDEBUG(AKMDBG_DISP1, "--------------------------------------------------------------------\n");
		AKMDEBUG(AKMDBG_DISP1, " Test No. Test Name    Fail    Test Data    [      Low         High]\n");
		AKMDEBUG(AKMDBG_DISP1, "--------------------------------------------------------------------\n");

		pf = 1;
	} else if ((testno == NULL) && (strncmp(testname, "END", 3) == 0)) {
		// Display result
		AKMDEBUG(AKMDBG_DISP1, "--------------------------------------------------------------------\n");
		if (*pf_total == 1) {
			AKMDEBUG(AKMDBG_DISP1, "Factory shipment test was passed.\n\n");
		} else {
			AKMDEBUG(AKMDBG_DISP1, "Factory shipment test was failed.\n\n");
		}

		pf = 1;
	} else {
		if ((lolimit <= testdata) && (testdata <= hilimit)) {
			//Pass
			pf = 1;
		} else {
			//Fail
			pf = -1;
		}

		//display result
		AKMDEBUG(AKMDBG_DISP1, " %7s  %-10s      %c    %9d    [%9d    %9d]\n",
				 testno, testname, ((pf == 1) ? ('.') : ('F')), testdata,
				 lolimit, hilimit);
	}

	//Pass/Fail check
	if (*pf_total != 0) {
		if ((*pf_total == 1) && (pf == 1)) {
			*pf_total = 1;            //Pass
		} else {
			*pf_total = -1;           //Fail
		}
	}
	return pf;
}

/*!
 Execute "Onboard Function Test" (NOT includes "START" and "END" command).
 @retval 1 The test is passed successfully.
 @retval -1 The test is failed.
 @retval 0 The test is aborted by kind of system error.
 */
int16 FST_AK09912(void)
{
	int16   pf_total;  //p/f flag for this subtest
	BYTE    i2cData[16];
	int16   hdata[3];
	int16   asax;
	int16   asay;
	int16   asaz;

	//***********************************************
	//  Reset Test Result
	//***********************************************
	pf_total = 1;

	//***********************************************
	//  Step1
	//***********************************************

	// Reset device.
	if (AKD_Reset() != AKD_SUCCESS) {
		AKMERROR;
		return 0;
	}

	// Read values from WIA.
	if (AKD_RxData(AK09912_REG_WIA1, i2cData, 2) != AKD_SUCCESS) {
		AKMERROR;
		return 0;
	}

	// TEST
	TEST_DATA(TLIMIT_NO_RST_WIA1,   TLIMIT_TN_RST_WIA1,   (int16)i2cData[0],  TLIMIT_LO_RST_WIA1,   TLIMIT_HI_RST_WIA1,   &pf_total);
	TEST_DATA(TLIMIT_NO_RST_WIA2,   TLIMIT_TN_RST_WIA2,   (int16)i2cData[1],  TLIMIT_LO_RST_WIA2,   TLIMIT_HI_RST_WIA2,   &pf_total);

	// Set to FUSE ROM access mode
	if (AKD_SetMode(AK09912_MODE_FUSE_ACCESS) != AKD_SUCCESS) {
		AKMERROR;
		return 0;
	}

	// Read values. CNTL
	if (AKD_RxData(AK09912_REG_CNTL2, i2cData, 1) != AKD_SUCCESS) {
		AKMERROR;
		return 0;
	}

	// TEST
	TEST_DATA(TLIMIT_NO_WR_CNTL2, TLIMIT_TN_WR_CNTL2, (int16)i2cData[0], TLIMIT_LO_WR_CNTL2, TLIMIT_HI_WR_CNTL2, &pf_total);

	// Read values from ASAX to ASAZ
	if (AKD_RxData(AK09912_FUSE_ASAX, i2cData, 3) != AKD_SUCCESS) {
		AKMERROR;
		return 0;
	}
	asax = (int16)i2cData[0];
	asay = (int16)i2cData[1];
	asaz = (int16)i2cData[2];

	// TEST
	TEST_DATA(TLIMIT_NO_ASAX, TLIMIT_TN_ASAX, asax, TLIMIT_LO_ASAX, TLIMIT_HI_ASAX, &pf_total);
	TEST_DATA(TLIMIT_NO_ASAY, TLIMIT_TN_ASAY, asay, TLIMIT_LO_ASAY, TLIMIT_HI_ASAY, &pf_total);
	TEST_DATA(TLIMIT_NO_ASAZ, TLIMIT_TN_ASAZ, asaz, TLIMIT_LO_ASAZ, TLIMIT_HI_ASAZ, &pf_total);

	// Set to PowerDown mode
	if (AKD_SetMode(AK09912_MODE_POWERDOWN) != AKD_SUCCESS) {
		AKMERROR;
		return 0;
	}

	//***********************************************
	//  Step2
	//***********************************************

	// Enable Temperature sensor
	i2cData[0] = AK09912_CNTL1_ENABLE_TEMP;
	if (AKD_TxData(AK09912_REG_CNTL1, i2cData, 1) != AKD_SUCCESS) {
		AKMERROR;
		return 0;
	}

	// Set to SNG measurement pattern (Set CNTL register)
	if (AKD_SetMode(AK09912_MODE_SNG_MEASURE) != AKD_SUCCESS) {
		AKMERROR;
		return 0;
	}

	// Wait for DRDY pin changes to HIGH.
	//usleep(AKM_MEASURE_TIME_US);
	// Get measurement data from AK09912
	// ST1 + (HXL + HXH) + (HYL + HYH) + (HZL + HZH) + TMPS + ST2
	// = 1 + (1 + 1) + (1 + 1) + (1 + 1) + 1 + 1 = 9Byte
	if (AKD_GetMagneticData(i2cData) != AKD_SUCCESS) {
		AKMERROR;
		return 0;
	}

	hdata[0] = (int16)((((uint16)(i2cData[2]))<<8)+(uint16)(i2cData[1]));
	hdata[1] = (int16)((((uint16)(i2cData[4]))<<8)+(uint16)(i2cData[3]));
	hdata[2] = (int16)((((uint16)(i2cData[6]))<<8)+(uint16)(i2cData[5]));

	// TEST
	i2cData[0] &= 0x7F;
	TEST_DATA(TLIMIT_NO_SNG_ST1,  TLIMIT_TN_SNG_ST1,  (int16)i2cData[0], TLIMIT_LO_SNG_ST1,  TLIMIT_HI_SNG_ST1,  &pf_total);
	
	// TEST
	TEST_DATA(TLIMIT_NO_SNG_HX,   TLIMIT_TN_SNG_HX,   hdata[0],          TLIMIT_LO_SNG_HX,   TLIMIT_HI_SNG_HX,   &pf_total);
	TEST_DATA(TLIMIT_NO_SNG_HY,   TLIMIT_TN_SNG_HY,   hdata[1],          TLIMIT_LO_SNG_HY,   TLIMIT_HI_SNG_HY,   &pf_total);
	TEST_DATA(TLIMIT_NO_SNG_HZ,   TLIMIT_TN_SNG_HZ,   hdata[2],          TLIMIT_LO_SNG_HZ,   TLIMIT_HI_SNG_HZ,   &pf_total);
	TEST_DATA(TLIMIT_NO_SNG_TMPS, TLIMIT_TN_SNG_TMPS, (int16)i2cData[7], TLIMIT_LO_SNG_TMPS, TLIMIT_HI_SNG_TMPS, &pf_total);
	TEST_DATA(TLIMIT_NO_SNG_ST2,  TLIMIT_TN_SNG_ST2,  (int16)i2cData[8], TLIMIT_LO_SNG_ST2,  TLIMIT_HI_SNG_ST2,  &pf_total);

	// Set to Self-test mode (Set CNTL register)
	if (AKD_SetMode(AK09912_MODE_SELF_TEST) != AKD_SUCCESS) {
		AKMERROR;
		return 0;
	}

	// Wait for DRDY pin changes to HIGH.
	//usleep(AKM_MEASURE_TIME_US);
	// Get measurement data from AK09912
	// ST1 + (HXL + HXH) + (HYL + HYH) + (HZL + HZH) + TMPS + ST2
	// = 1 + (1 + 1) + (1 + 1) + (1 + 1) + 1 + 1 = 9Byte
	if (AKD_GetMagneticData(i2cData) != AKD_SUCCESS) {
		AKMERROR;
		return 0;
	}

	// TEST
	i2cData[0] &= 0x7F;
	TEST_DATA(TLIMIT_NO_SLF_ST1, TLIMIT_TN_SLF_ST1, (int16)i2cData[0], TLIMIT_LO_SLF_ST1, TLIMIT_HI_SLF_ST1, &pf_total);

	hdata[0] = (int16)((((uint16)(i2cData[2]))<<8)+(uint16)(i2cData[1]));
	hdata[1] = (int16)((((uint16)(i2cData[4]))<<8)+(uint16)(i2cData[3]));
	hdata[2] = (int16)((((uint16)(i2cData[6]))<<8)+(uint16)(i2cData[5]));

	// TEST
	TEST_DATA(
			  TLIMIT_NO_SLF_RVHX,
			  TLIMIT_TN_SLF_RVHX,
			  (hdata[0])*((asax - 128)*0.5f/128.0f + 1),
			  TLIMIT_LO_SLF_RVHX,
			  TLIMIT_HI_SLF_RVHX,
			  &pf_total
			  );

	TEST_DATA(
			  TLIMIT_NO_SLF_RVHY,
			  TLIMIT_TN_SLF_RVHY,
			  (hdata[1])*((asay - 128)*0.5f/128.0f + 1),
			  TLIMIT_LO_SLF_RVHY,
			  TLIMIT_HI_SLF_RVHY,
			  &pf_total
			  );

	TEST_DATA(
			  TLIMIT_NO_SLF_RVHZ,
			  TLIMIT_TN_SLF_RVHZ,
			  (hdata[2])*((asaz - 128)*0.5f/128.0f + 1),
			  TLIMIT_LO_SLF_RVHZ,
			  TLIMIT_HI_SLF_RVHZ,
			  &pf_total
			  );

		TEST_DATA(
			TLIMIT_NO_SLF_ST2,
			TLIMIT_TN_SLF_ST2,
			(int16)i2cData[8],
			TLIMIT_LO_SLF_ST2,
			TLIMIT_HI_SLF_ST2,
			&pf_total
			);

	return pf_total;
}

/*!
 Execute "Onboard Function Test" (includes "START" and "END" command).
 @retval 1 The test is passed successfully.
 @retval -1 The test is failed.
 @retval 0 The test is aborted by kind of system error.
 */
int16 FST_Body(void)
{
	int16 pf_total = 1;

	//***********************************************
	//    Reset Test Result
	//***********************************************
	TEST_DATA(NULL, "START", 0, 0, 0, &pf_total);

	//***********************************************
	//    Step 1 to 2
	//***********************************************
	pf_total = FST_AK09912();

	//***********************************************
	//    Judge Test Result
	//***********************************************
	TEST_DATA(NULL, "END", 0, 0, 0, &pf_total);

	return pf_total;
}


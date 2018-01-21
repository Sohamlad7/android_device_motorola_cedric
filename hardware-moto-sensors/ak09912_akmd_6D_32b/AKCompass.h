/******************************************************************************
 *
 * $Id: AKCompass.h 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#ifndef AKMD_INC_AKCOMPASS_H
#define AKMD_INC_AKCOMPASS_H

#include "AKCommon.h"
#include "CustomerSpec.h"
#include "AKMD_Driver.h" // for using BYTE

//**************************************
// Include files for SmartCompass library.
//**************************************
#include "libSmartCompass/AKCertification.h"
#include "libSmartCompass/AKConfigure.h"
#include "libSmartCompass/AKDecomp.h"
#include "libSmartCompass/AKMDevice.h"
#include "libSmartCompass/AKDirection6D.h"
#include "libSmartCompass/AKHDOE.h"
#include "libSmartCompass/AKHFlucCheck.h"
#include "libSmartCompass/AKManualCal.h"
#include "libSmartCompass/AKVersion.h"


/*** Constant definition ******************************************************/
#define	THETAFILTER_SCALE	4128
#define	HFLUCV_TH			2500
#define PDC_SIZE			27

/*** Type declaration *********************************************************/
typedef enum _AKMD_PATNO {
	PAT_INVALID = 0,
	PAT1,
	PAT2,
	PAT3,
	PAT4,
	PAT5,
	PAT6,
	PAT7,
	PAT8
} AKMD_PATNO;


/*! A parameter structure which is needed for HDOE and Direction calculation. */
typedef struct _AKSCPRMS{

	// Variables for magnetic sensor.
	int16vec	m_ho;
	int16vec	HSUC_HO[CSPEC_NUM_FORMATION];
	int32vec	m_ho32;
	int16vec	m_hs;
	int16vec	HFLUCV_HREF[CSPEC_NUM_FORMATION];
	AKSC_HFLUCVAR	m_hflucv;

	// Variables for DecompS3 .
	int16vec	m_hdata[AKSC_HDATA_SIZE];
	int16		m_hn;		// Number of acquired data
	int16vec	m_hvec;		// Averaged value
	int16vec	m_asa;
	uint8		m_pdc[PDC_SIZE];
	uint8*		m_pdcptr;

	// Variables for HDOE.
	AKSC_HDOEVAR	m_hdoev;
	AKSC_HDST		m_hdst;
	AKSC_HDST		HSUC_HDST[CSPEC_NUM_FORMATION];

	// Variables for formation change
	int16		m_form;
	int16		m_cntSuspend;

	// Variables for Direction9D.
	int16		m_d6dRet;
	int16		m_hnave;
	int16vec	m_dvec;
	int16		m_theta;
	int16		m_delta;
	int16		m_hr;
	int16		m_hrhoriz;
	int16		m_ar;
	int16		m_phi180;
	int16		m_phi90;
	int16		m_eta180;
	int16		m_eta90;
	I16MATRIX	m_mat;
	I16QUAT		m_quat;

	// Variables for acceleration sensor.
	int16vec	m_AO;
	int16vec	m_avec;

	// Layout information
	AKMD_PATNO	m_hlayout;
	//AKMD_PATNO	m_alayout;
	//AKMD_PATNO	m_glayout;

	// Variables for decimation.
	int16		m_callcnt;

	// Ceritication
	uint8		m_licenser[AKSC_CI_MAX_CHARSIZE+1];	//end with '\0'
	uint8		m_licensee[AKSC_CI_MAX_CHARSIZE+1];	//end with '\0'
	int16		m_key[AKSC_CI_MAX_KEYSIZE];

	// base
	int32vec	m_hbase;
	int32vec	HSUC_HBASE[CSPEC_NUM_FORMATION];

} AKSCPRMS;


/*** Global variables *********************************************************/

/*** Prototype of function ****************************************************/

#endif //AKMD_INC_AKCOMPASS_H


/******************************************************************************
 *
 * $Id: CustomerSpec.h 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#ifndef AKMD_INC_CUSTOMERSPEC_H
#define AKMD_INC_CUSTOMERSPEC_H

/*******************************************************************************
 User defines parameters.
 ******************************************************************************/

// Certification information
#define CSPEC_CI_LICENSER	"ASAHIKASEI"
#define CSPEC_CI_LICENSEE	"MOTOROLA"

// Parameters for Average
//	The number of magnetic data block to be averaged.
//	 NBaveh*(*nh) must be 1, 2, 4, 8 or 16.
#define CSPEC_HNAVE		8

// Parameters for Direction Calculation
#define CSPEC_DVEC_X		0
#define CSPEC_DVEC_Y		0
#define CSPEC_DVEC_Z		0

// The number of formation
#define CSPEC_NUM_FORMATION		1

// the counter of Suspend
#define CSPEC_CNTSUSPEND_SNG	8

//Parameters for Temperature sensor
//	0x00 : disable
//	0x80 : enable
#define CSPEC_TEMPERATURE		0x80

//Parameters for Noise Suppression Filter
//	0x00 : disable
//	0x20 : Low
//	0x40 : Middle
//	0x60 : High
#define CSPEC_NSF				0x40

// Setting file
#define CSPEC_SETTING_FILE	"/data/misc/akmd/akmd_set.txt"
#define CSPEC_PDC_FILE		"/data/misc/akmd/pdc.txt"

#endif //AKMD_INC_CUSTOMERSPEC_H


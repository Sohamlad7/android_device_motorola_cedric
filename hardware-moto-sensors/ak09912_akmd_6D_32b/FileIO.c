/******************************************************************************
 *
 * $Id: FileIO.c 983 2013-02-08 04:43:03Z miyazaki.hr $
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
#include "FileIO.h"
#include <sys/stat.h>

#define AKM_PERM (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)

/*!
 Load parameters from file which is specified with #CSPEC_SETTING_FILE.
 This function reads data from a beginning of the file line by line, and
 check parameter name sequentially. In otherword, this function depends on
 the order of eache parameter described in the file.
 @return If function fails, the return value is 0. When function fails, the
 output is undefined. Therefore, parameters which are possibly overwritten
 by this function should be initialized again. If function succeeds, the
 return value is 1.
 @param[out] prms A pointer to #AKSCPRMS structure. Loaded parameter is
 stored to the member of this structure.
 */
int16 LoadParameters(AKSCPRMS * prms)
{
	int16	i, ret;
	int		tmp;
	FILE	*fp = NULL;
	char	keyName[KEYNAME_SIZE];

	//Open setting file for read.
	if ((fp = fopen(CSPEC_SETTING_FILE, "r")) == NULL) {
		AKMERROR_STR("fopen");
		return 0;
	}

	ret = 1;

	// Load data to HDST, HO, HREF, THRE
	for (i = 0; i < CSPEC_NUM_FORMATION; i++) {
		snprintf(keyName, sizeof(keyName), "HSUC_HDST_FORM%d", i);
		tmp = 0;
		ret = ret && LoadInt(fp, keyName, &tmp);
		prms->HSUC_HDST[i] = (AKSC_HDST)tmp;

		snprintf(keyName, sizeof(keyName), "HSUC_HO_FORM%d", i);
		ret = ret && LoadInt16vec(fp, keyName, &prms->HSUC_HO[i]);

		snprintf(keyName, sizeof(keyName), "HFLUCV_HREF_FORM%d", i);
		ret = ret && LoadInt16vec(fp, keyName, &prms->HFLUCV_HREF[i]);

		snprintf(keyName, sizeof(keyName), "HSUC_HBASE_FORM%d", i);
		ret = ret && LoadInt32vec(fp, keyName, &prms->HSUC_HBASE[i]);
	}

	// Offset of other sensors.
	ret = ret && LoadInt16vec(fp, "AO", &prms->m_AO);

	if (fclose(fp) != 0) {
		AKMERROR_STR("fclose");
		ret = 0;
	}

	if (ret == 0) {
		AKMERROR;
	}
	return ret;
}

/*! Load PDC from file named with #SETTING_PDC_FILE_NAME.
  This function reads parameters from a beginning of the file line by line,
  and check parameter name sequentially.

  @return When function fails, the return value is 0. In that case, all
  related parameters, i.e. m_pdc and m_pdcptr, are initialized with 0.
*/
int16 LoadPDC(AKSCPRMS* prms)
{
	int16   ret, i;
	FILE*   fp;
	char    keyName[KEYNAME_SIZE];
	char    header[HEADER_SIZE];
	int		tmp;

	ret = 1;

	//Open setting file for read.
	if((fp = fopen(CSPEC_PDC_FILE, "r")) == NULL){
		AKMERROR_STR("fopen");
		goto PDCREAD_FAILED;
	}

	//Read file header
	if(fgets(header, sizeof(header), fp) == NULL){
		AKMERROR_STR("fgets");
		fclose(fp);
		goto PDCREAD_FAILED;
	}

	//Set loaded data to pdc.
	for (i = 0; i<PDC_SIZE; i++) {
		snprintf(keyName, sizeof(keyName), "HPRMS%d", i);
		tmp = 0;
		ret = ret && LoadInt(fp, keyName, &tmp);
		prms->m_pdc[i] = (uint8)tmp;
	}

	if (fclose(fp) != 0) {
		AKMERROR_STR("fclose");
		goto PDCREAD_FAILED;
	}

	if(ret != 1){
		AKMERROR;
		goto PDCREAD_FAILED;
	}

	// Set parameter's pointer.
	prms->m_pdcptr = prms->m_pdc;

	AKMDEBUG(AKMDBG_DEBUG, "Parameter loaded: %s\n", header);

	return  1;

PDCREAD_FAILED:
	memset(prms->m_pdc, 0, sizeof(prms->m_pdc));
	prms->m_pdcptr = 0;

	return 0;
}

/*!
 Load \c int type value from file. The name of parameter is specified with
 \a lpKeyName. If the name matches the beginning of read line, the string
 after #DELIMITER will converted to \c int type value.
 @return If function fails, the return value is 0. When function fails, the
 value @ val is not overwritten. If function succeeds, the return value is 1.
 @param[in] fp Pointer to \c FILE structure.
 @param[in] lpKeyName The name of parameter.
 @param[out] val Pointer to \c int type value. Upon successful completion
 of this function, read value is copied to this variable.
 */
int16 LoadInt(FILE * fp, const char *lpKeyName, int *val)
{
	char buf[KEYNAME_SIZE] = { '\0' };

	// ATTENTION! %ns should be modified according to the size of buf.
	if (fscanf(fp, "%63s" DELIMITER "%11d", buf, val) != 2) {
		AKMERROR;
		return 0;
	}
	// Compare the read parameter name with given name.
	if (strncmp(buf, lpKeyName, sizeof(buf)) != 0) {
		ALOGE("%s: strncmp (%s) error.", __FUNCTION__, lpKeyName);
		return 0;
	}

	return 1;
}

/*!
 Load \c int16vec type value from file and convert it to int16vec type
 structure. This function adds ".x", ".y" and ".z" to the last of parameter
 name and try to read value with combined name.
 @return If function fails, the return value is 0. When function fails, the
 output is undefined. If function succeeds, the return value is 1.
 @param[in] fp A opened \c FILE pointer.
 @param[in] lpKeyName The parameter name.
 @param[out] vec A pointer to int16vec structure. Upon successful completion
 of this function, read values are copied to this variable.
 */
int16 LoadInt16vec(FILE * fp, const char *lpKeyName, int16vec * vec)
{
	char	keyName[KEYNAME_SIZE];
	int16	ret = 1;
	int		tmp;

	snprintf(keyName, sizeof(keyName), "%s.x", lpKeyName);
	ret = ret && LoadInt(fp, keyName, &tmp);
	vec->u.x = (int16)tmp;

	snprintf(keyName, sizeof(keyName), "%s.y", lpKeyName);
	ret = ret && LoadInt(fp, keyName, &tmp);
	vec->u.y = (int16)tmp;

	snprintf(keyName, sizeof(keyName), "%s.z", lpKeyName);
	ret = ret && LoadInt(fp, keyName, &tmp);
	vec->u.z = (int16)tmp;

	return ret;
}

/*!
 Load \c int32vec type value from file and convert it to int32vec type
 structure. This function adds ".x", ".y" and ".z" to the last of parameter
 name and try to read value with combined name.
 @return If function fails, the return value is 0. When function fails, the
 output is undefined. If function succeeds, the return value is 1.
 @param[in] fp A opened \c FILE pointer.
 @param[in] lpKeyName The parameter name.
 @param[out] vec A pointer to int32vec structure. Upon successful completion
 of this function, read values are copied to this variable.
 */
int16 LoadInt32vec(FILE * fp, const char *lpKeyName, int32vec * vec)
{
	char	keyName[KEYNAME_SIZE];
	int16	ret = 1;
	int		tmp;

	snprintf(keyName, sizeof(keyName), "%s.x", lpKeyName);
	ret = ret && LoadInt(fp, keyName, &tmp);
	vec->u.x = (int32)tmp;

	snprintf(keyName, sizeof(keyName), "%s.y", lpKeyName);
	ret = ret && LoadInt(fp, keyName, &tmp);
	vec->u.y = (int32)tmp;

	snprintf(keyName, sizeof(keyName), "%s.z", lpKeyName);
	ret = ret && LoadInt(fp, keyName, &tmp);
	vec->u.z = (int32)tmp;

	return ret;
}


/*!
 Save parameters to file which is specified with #CSPEC_SETTING_FILE. This
 function saves variables when the offsets of magnetic sensor estimated
 successfully.
 @return If function fails, the return value is 0. When function fails, the
 parameter file may collapsed. Therefore, the parameters file should be
 discarded. If function succeeds, the return value is 1.
 @param[out] prms A pointer to #AKSCPRMS structure. Member variables are
 saved to the parameter file.
 */
int16 SaveParameters(AKSCPRMS * prms)
{
	int16	ret = 1;
	int16	i;
	FILE	*fp;
	char	keyName[KEYNAME_SIZE];

	//Open setting file for write.
	if ((fp = fopen(CSPEC_SETTING_FILE, "w")) == NULL) {
		AKMERROR_STR("fopen");
		return 0;
	}

	for (i = 0; i < CSPEC_NUM_FORMATION; i++) {
		snprintf(keyName, sizeof(keyName), "HSUC_HDST_FORM%d", i);
		ret = ret && SaveInt(fp, keyName, (int)prms->HSUC_HDST[i]);

		snprintf(keyName, sizeof(keyName), "HSUC_HO_FORM%d", i);
		ret = ret && SaveInt16vec(fp, keyName, &prms->HSUC_HO[i]);

		snprintf(keyName, sizeof(keyName), "HFLUCV_HREF_FORM%d", i);
		ret = ret && SaveInt16vec(fp, keyName, &prms->HFLUCV_HREF[i]);

		snprintf(keyName, sizeof(keyName), "HSUC_HBASE_FORM%d", i);
		ret = ret && SaveInt32vec(fp, keyName, &prms->HSUC_HBASE[i]);
	}

	// Offset of other sensors.
	ret = ret && SaveInt16vec(fp, "AO", &prms->m_AO);

	if (fclose(fp) != 0) {
		AKMERROR_STR("fclose");
		ret = 0;
	}
    if (chmod(CSPEC_SETTING_FILE, AKM_PERM) != 0) {
		AKMERROR_STR("chmod");
		ret = 0;
	}

	if (ret == 0) {
		AKMERROR;
	}

	return ret;
}

/*!
 Save parameters of int16 type structure to file. This function adds
 @return If function fails, the return value is 0. When function fails,
 parameter is not saved to file. If function succeeds, the return value is 1.
 @param[in] fp Pointer to \c FILE structure.
 @param[in] lpKeyName The name of paraemter.
 @param[in] val Pointer to \c int16 type value.
 */
int16 SaveInt(FILE * fp, const char *lpKeyName, const int val)
{
	if (fprintf(fp, "%s" DELIMITER "%d\n", lpKeyName, val) < 0) {
		ALOGE("%s: printf (%s) error.", __FUNCTION__, lpKeyName);
		return 0;
	} else {
		return 1;
	}
}

/*!
 Save parameters of int16vec type structure to file. This function adds
 ".x", ".y" and ".z" to the last of parameter name and save value with
 the combined name.
 @return If function fails, the return value is 0. When function fails, not
 all parameters are saved to file, i.e. parameters file may collapsed.
 If function succeeds, the return value is 1.
 @param[in] fp Pointer to \c FILE structure.
 @param[in] lpKeyName The name of paraemter.
 @param[in] vec Pointer to \c int16vec type structure.
 */
int16 SaveInt16vec(FILE * fp, const char *lpKeyName,
				   const int16vec * vec)
{
	int16 ret = 0;
	char keyName[KEYNAME_SIZE];

	ret = 1;
	snprintf(keyName, sizeof(keyName), "%s.x", lpKeyName);
	ret = ret && SaveInt(fp, keyName, vec->u.x);

	snprintf(keyName, sizeof(keyName), "%s.y", lpKeyName);
	ret = ret && SaveInt(fp, keyName, vec->u.y);

	snprintf(keyName, sizeof(keyName), "%s.z", lpKeyName);
	ret = ret && SaveInt(fp, keyName, vec->u.z);

	return ret;
}

/*!
 Save parameters of int32vec type structure to file. This function adds
 ".x", ".y" and ".z" to the last of parameter name and save value with
 the combined name.
 @return If function fails, the return value is 0. When function fails, not
 all parameters are saved to file, i.e. parameters file may collapsed.
 If function succeeds, the return value is 1.
 @param[in] fp Pointer to \c FILE structure.
 @param[in] lpKeyName The name of paraemter.
 @param[in] vec Pointer to \c int32vec type structure.
 */
int16 SaveInt32vec(FILE * fp, const char *lpKeyName,
				   const int32vec * vec)
{
	int16 ret = 0;
	char keyName[KEYNAME_SIZE];

	ret = 1;
	snprintf(keyName, sizeof(keyName), "%s.x", lpKeyName);
	ret = ret && SaveInt(fp, keyName, vec->u.x);

	snprintf(keyName, sizeof(keyName), "%s.y", lpKeyName);
	ret = ret && SaveInt(fp, keyName, vec->u.y);

	snprintf(keyName, sizeof(keyName), "%s.z", lpKeyName);
	ret = ret && SaveInt(fp, keyName, vec->u.z);

	return ret;
}


/*
 * Copyright (c) 2014, NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef INCLUDED_BCT_PRIVATE_H
#define INCLUDED_BCT_PRIVATE_H

#include "bct.h"

enum { BCT_MAX_BL_PARTITION_ID = 255 };

enum { BCT_MAX_BL_VERSION = 255 };

/**
* @brief Returns BCT length
*
*/
uint32_t bct_priv_get_size(void);

/**
 * @brief Retrieves size of the data element  as well as the
 * number of instances of the data element present in the BCT.
 *
 * @param hbct Bct handle for bct manipulation
 * @param data_type type of data to obtain from BCT
 * @param psize pointer to size of Data buffer
 * @param pnum_instances pointer to instance number
 *
 * @return NO_ERROR BCT data element retrieved successfully
 * @return ERR_NOT_SUPPORTED Unknown data type, or illegal instance number
 *
 */
status_t
bct_priv_getdata_size(
	bct_handle hbct,
	bct_data_type data_type,
	uint32_t *psize,
	uint32_t *pnum_instances);

/**
 * @brief  Retrieve data from the BCT.
 *
 * @param hbct  handle to BCT instance
 * @param data_type Type of data to obtain from BCT
 * @param psize  Pointer to size of Data buffer
 * @param pinstance Pointer to instance number of the data element to retrieve
 * @param pdata  Buffer for storing the retrieved instance of the data element
 *
 * @return NO_ERROR  BCT data element retrieved successfully
 */
status_t
bct_priv_get_data(
	bct_handle hbct,
	bct_data_type data_type,
	uint32_t *psize,
	uint32_t *pinstance,
	void *pdata);

/**
 * @brief  Set data in the BCT.
 *
 * @param hbct handle to BCT instance
 * @param data_type type of data to set in the BCT
 * @param psize pointer to size of Data buffer
 * @param pinstance pointer to the instance number of the data element to set
 * @param pdata buffer for storing the desired value of the data element
 *
 * @return NO_ERROR BCT data element set successfully
 */
status_t
bct_priv_set_data(
	bct_handle hbct,
	bct_data_type data_type,
	uint32_t *psize,
	uint32_t *pinstance,
	void *pdata);

/**
 * @brief Get the offset of the signature in BCT
 *
 * @return Offset of the Signature
 */
uint32_t bct_get_signdata_offset_priv(void);

#endif // INCLUDED_BCT_PRIVATE_H


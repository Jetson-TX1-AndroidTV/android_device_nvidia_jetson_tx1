/*
 * Copyright (c) 2014-2015, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include <err.h>
#include <bct.h>
#include <bct_private.h>
#include <platform/nvboot/nvboot_bit.h>

#define IRAM_START_ADDRESS 0x40000000UL

uint32_t get_bct_size(void)
{
	return bct_priv_get_size();
}

status_t
bct_init(
	uint32_t *psize,
	void *pbuffer,
	bct_handle *phbct)
{
	uint32_t bct_size = 0;
	bct_handle htmp_handle = NULL;

	bct_size = bct_priv_get_size();

	if (!phbct) {

		if (psize && *psize == 0) {
			*psize = bct_size;
			return -1;
		}

		return -1;
	}

	if (pbuffer) {
		if (!psize)
			return -1;

		if (*psize < bct_size)
			return -1;

		htmp_handle = pbuffer;
	} else {
			NvBootInfoTable *bitptr = (NvBootInfoTable *)IRAM_START_ADDRESS;
			htmp_handle = (bct_handle)(uintptr_t)(bitptr->BctPtr);
	}
	*phbct = htmp_handle;
	return 0;
}

void bct_deinit(bct_handle hBct)
{
}

status_t
bct_get_data(
	bct_handle hbct,
	bct_data_type data_type,
	uint32_t *psize,
	uint32_t *pinstance,
	void *pdata)
{
	uint32_t min_data_size;
	uint32_t num_instances;
	status_t e = NO_ERROR;

	if (!hbct)
		return ERR_INVALID_ARGS;

	if (!psize || !pinstance)
		return ERR_INVALID_ARGS;

	e = bct_priv_getdata_size(hbct, data_type, &min_data_size, &num_instances);
	if (e != NO_ERROR) {
		return  ERR_NOT_SUPPORTED;
	}

	if (*psize == 0) {
		*psize = min_data_size;
		*pinstance = num_instances;

	if (pdata)
		return ERR_NOT_ENOUGH_BUFFER;
	else
		return NO_ERROR;
	}

	*psize = min_data_size;

	if (*pinstance > num_instances)
		return ERR_INVALID_ARGS;

	if (!pdata)
		return ERR_INVALID_ARGS;

	e = bct_priv_get_data(hbct, data_type, psize, pinstance, pdata);
	if (e != NO_ERROR) {
		return  ERR_NOT_SUPPORTED;
	}

	return e;
}

status_t
bct_set_data(
    bct_handle hbct,
    bct_data_type data_type,
	uint32_t *psize,
	uint32_t *pinstance,
	void *pdata)
{
	uint32_t min_data_size;
	uint32_t num_instances;
	status_t e = NO_ERROR;

	if (!hbct)
		return ERR_INVALID_ARGS;

	if (!psize || !pinstance)
		return ERR_INVALID_ARGS;

	e = bct_priv_getdata_size(hbct, data_type, &min_data_size, &num_instances);
	if (e != NO_ERROR) {
		return  ERR_NOT_SUPPORTED;
	}

	if (*psize == 0) {
		*psize = min_data_size;
		*pinstance = num_instances;

		if (pdata)
			return ERR_NOT_ENOUGH_BUFFER;
		else
			return NO_ERROR;
	}

	*psize = min_data_size;

	if (*pinstance > num_instances)
		return ERR_INVALID_ARGS;

	if (!pdata)
		return ERR_INVALID_ARGS;
	e = bct_priv_set_data(hbct, data_type, psize, pinstance, pdata);
	if (e != NO_ERROR) {
		return  ERR_NOT_SUPPORTED;
	}

	return e;
}

uint32_t bct_get_signdata_offset(void)
{
	return bct_get_signdata_offset_priv();
}

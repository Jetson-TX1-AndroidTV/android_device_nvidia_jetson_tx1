/*
 * Copyright (c) 2014-2016, NVIDIA Corporation. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

/**
 * @file
 * <b>NVIDIA Boot Configuration Table Management Framework</b>
 *
 * @b Description: This file declares the APIs for interacting with
 *    the Boot Configuration Table (BCT).
 */

#ifndef INCLUDED_BCT_H
#define INCLUDED_BCT_H

#include <sys/types.h>
#include <stdbool.h>

#define NO_BUILD_CONFIG

typedef int status_t;

/**
 * Defines data elements of the BCT that can be queried and set via
 * the Bct API.
 */
typedef enum
{
	/* Version of BCT structure */
	bct_data_type_version,

	/* Device parameters with which the secondary boot device controller is
		initialized */
	bct_data_type_bootdevice_config_info,

	/* Number of valid device parameter sets */
	bct_data_type_num_valid_bootdevice_configs,

	/* SDRAM parameters with which the SDRAM controller is initialized */
	bct_data_type_sdram_configinfo,

	/* Number of valid SDRAM parameter sets */
    bct_data_type_num_valid_sdramconfigs,

	/* Generic Attribute for specified BL instance */
	bct_data_type_bootloader_attribute,

	/* Version number for specified BL instance */
	bct_data_type_bootloader_version,

	/* Partition id of the BCT */
	bct_data_type_bct_partitionid,

	/* Start Block for specified BL instance */
	bct_data_type_bootloader_startblock,

	/* Start Sector for specified BL instance */
	bct_data_type_bootloader_start_sector,

	/* Length (in bytes) of specified BL instance */
	bct_data_type_bootloader_length,

	/* Load Address for specified BL instance */
	bct_data_type_bootloader_loadaddress,

	/* Entry Point for specified BL instance */
	bct_data_type_bootloader_entrypoint,

	/* Crypto Hash for specified BL instance */
	bct_data_type_bootloader_cryptohash,

	/* Number of Boot Loaders that are enabled for booting */
	bct_data_type_num_enabled_bootloaders,

	/* Bad block table */
    bct_data_type_badblock_table,

	/* Partition size */
	bct_data_type_partition_size,

	/* Specifies the size of a physical block on the secondary boot device
		in log2(bytes). */
	bct_data_type_bootdevice_blocksizelog2,

	/* Specifies the size of a page on the secondary boot device in log2(bytes).*/
	bct_data_type_bootdevice_pagesizelog2,

	/* Specifies a region of data available to customers of the BR.  This data
		region is primarily used by a manufacturing utility or BL to store
		useful information that needs to be shared among manufacturing utility,
		BL, and OS image.  BR only provides framework and does not use this
	data.

	@note Some of this space has already been allocated for use
	by NVIDIA. */
	bct_data_type_auxdata,
	bct_data_type_auxdata_aligned,
	/* Version of customer data stored in BCT */
	bct_data_type_customer_dataversion,
	/* Boot device type stored in BCT customer data */
	bct_data_type_devparams_type,

	/* Specifies the signature for the BCT structure.*/
	bct_data_type_cryptohash,

	/* Specifies random data used in lieu of a formal initial vector when
		encrypting and/or computing a hash for the BCT. */
	bct_data_type_cryptosalt,

	/* Specifies extent of BCT data to be hashed */
	bct_data_type_hashdata_offset,
	bct_data_type_hashdata_length,

	/* Customer defined configuration parameter */
	bct_data_type_odmoption,

	/* Specifies entire BCT */
	bct_data_type_fullcontents,

	/* Specifies size of bct */
	bct_data_type_bct_size,

	bct_data_Type_num,

	/* Specifies a reserved area at the end of the BCT that must be
		filled with the padding pattern. */
    bct_data_type_reserved,

	/* Specifies the type of device for parameter set DevParams[i]. */
	bct_data_type_devtype,
	/* Partition Hash Enums */
	bct_data_type_hashedpartition_partid,
	/* Crypto Hash for specified Partition */
    bct_data_type_hashedpartition_cryptohash,
	/* Enable Failback */
	bct_data_type_enable_failback,
	/* One time flashable infos */
	bct_data_type_internalinfo_onetimeraw,
	bct_data_type_internalinfo_version,

	bct_data_type_rsakey_modulus,
	bct_data_type_rsa_psssig,
	bct_data_type_bootloader_rsa_psssig,
	bct_data_type_num_valid_devtype,
	bct_data_type_unique_chipid,

	/* Specifies the signed section of the BCT. */
	bct_data_type_signedsection,
#if IS_T132
	/* Generic Attribute for specified MTS instance */
	bct_data_type_mts_attribute,
	/* Version number for specified MTS instance */
	bct_data_type_mts_version,
	/* Start Block for specified MTS instance */
	bct_data_type_mts_startblock,
	/* Start Sector for specified MTS instance */
	bct_data_type_mts_startsector,
	/* Length (in bytes) of specified MTS instance */
	bct_data_type_mts_length,
	/* Load Address for specified MTS instance */
	bct_data_type_mts_loadaddress,
	/* Entry Point for specified MTS instance */
	bct_data_type_mts_entrypoint,
	/* Number of MTS Loaders that are enabled for booting */
	bct_data_type_num_enabledmts,
#endif

	/* Specifies the aux info stored in customer data region */
	bct_data_type_auxinfo,

	bct_data_type_secure_jtagcontrol,

	/* The address for dump ram carveout*/
	bct_data_type_ramdump_carveout,

	/* Start Sector for specified BFS instance */
	bct_data_type_bootfileset_start_sector,

	/* Length for specified BFS instance */
	bct_data_type_bootfileset_length,

	/* Chksum for specified BFS instance */
	bct_data_type_bootfileset_chksum,

	/* Chksum for specified BFS instance */
	bct_data_type_bootfileset_chksum_length,

	bct_data_type_max = 0x7fffffff
} bct_data_type;

typedef enum
{
	bct_boot_device_nand8,
	bct_boot_device_emmc,
	bct_boot_device_spi_nor,
	bct_boot_device_nand16,
	bct_boot_device_Current,
	bct_boot_device_num,
	bct_boot_device_max = 0x7fffffff
} bct_boot_device;


typedef struct bct_rec *bct_handle;

/**
 * Gets size of the BCT from offset 0.
 *
 * @return The size
  */
 uint32_t get_bct_size(void);

/**
 * Create a handle to the specified BCT.
 *
 * The supplied buffer must contain a BCT, or if Buffer is NULL then the
 * system's default BCT (the BCT used by the Boot ROM to boot the chip) will be
 * used.
 *
 * This routine must be called before any of the other Bct*() API's.  The
 * handle must be passed as an argument to all subsequent Bct*() invocations.
 *
 * The size of the BCT can be queried by specifying a NULL phBct and a non-NULL
 * size.  In this case, the routine will set *Size equal to the BCT size and
 * return NO_ERROR.
 *
 * @param size size of Buffer, in bytes
 * @param buffer buffer containing a BCT, or NULL to select the system's default
 *        BCT
 * @param phbct pointer to handle for invoking subsequent operations on the BCT;
 *        can be specified as NULL to query BCT size
 *
 * @retval NO_ERROR BCT handle creation successful or BCT size query successful
 * @retval ERR_INVALID_ARGS Illegal pointer value
 * @retval ERR_NOT_ENOUGH_BUFFER Not enough memory to allocate handle
 */
status_t bct_init(
	uint32_t *size,
	void *buffer,
	bct_handle *phbct);

/**
 * Release BCT handle and associated resources (but not the BCT itself).
 *
 * @param phbct handle to BCT instance
 */
void bct_deinit(bct_handle phbct);

/**
 * Retrieve data from the BCT.
 *
 * This API allows the size of the data element to be retrieved, as well as the
 * number of instances of the data element present in the BCT.
 *
 * To retrieve that value of a given data element, allocate a Data buffer large
 * enough to hold the requested data, set *size to the size of the buffer, and
 * set *instance to the instance number of interest.
 *
 * To retrieve the size and number of instances of a given type of data element,
 * specified a *size of zero and a non-NULL Instance (pointer).  As a result,
 * bct_get_data() will set *Size to the actual size of the data element and
 * *instance to the number of available instances of the data element in the
 * BCT, then report NO_ERROR.
 *
 * @param hbct handle to BCT instance
 * @param data_type type of data to obtain from BCT
 * @param psize pointer to size of Data buffer; set *Size to zero in order to
 *        retrieve the data element size and number of instances
 * @param pinstance pointer to instance number of the data element to retrieve
 * @param pdata buffer for storing the retrieved instance of the data element
 *
 * @retval NO_ERROR BCT data element retrieved successfully
 * @retval ERR_NOT_ENOUGH_BUFFER data buffer (*Size) too small
 * @retval ERR_INVALID_ARGS Illegal pointer value
 */
status_t
bct_get_data(
	bct_handle hbct,
	bct_data_type data_type,
	uint32_t *psize,
	uint32_t *pinstance,
	void *pdata);

/**
 * Set data in the BCT.
 *
 * This API works like bct_get_data() to retrieve the size and number of
 * instances of the data element in the BCT.
 *
 * To set the value of a given instance of the data element, set *Size to the size
 * of the data element, *Instance to the desired instance, and provide the data
 * value in the Data buffer.
 *
 * @param hbct handle to BCT instance
 * @param data_type type of data to set in the BCT
 * @param psize pointer to size of Data buffer; set *Size to zero in order to
 *        retrieve the data element size and number of instances
 * @param pinstance pointer to the instance number of the data element to set
 * @param pdata buffer for storing the desired value of the data element
 *
 * @retval NO_ERROR BCT data element set successfully
 * @retval ERR_NOT_ENOUGH_BUFFER data buffer (*Size) too small
 * @retval ERR_INVALID_ARGS Illegal pointer value
 */
status_t
bct_set_data(
	bct_handle hBct,
	bct_data_type data_type,
	uint32_t *psize,
	uint32_t *pinstance,
	void *pdata);

/**
 * @brief Gets the offset of signed section in BCT
 *
 * @return Returns Offset
 */
uint32_t bct_get_signdata_offset(void);

#endif // INCLUDED__BCT_H

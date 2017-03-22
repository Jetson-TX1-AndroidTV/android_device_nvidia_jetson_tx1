/*
 * Copyright (c) 2014-2016, NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */


#include <bct_private.h>
#include <bct.h>
#include <err.h>
#include <string.h>
#include <assert.h>
#include <platform/nvboot/nvboot_bct.h>

#define BCT_LENGTH sizeof(NvBootConfigTable)
#define MOD_SIZE ARSE_RSA_MAX_MODULUS_SIZE / 8 / 4

#define RSA_KEY_SIZE 256

/* Add assert in below macro */
#define PACK_uint32(Data32, pData8) \
	do { \
		(Data32) = ((pData8)[3] << 24) | \
				((pData8)[2] << 16) | \
				((pData8)[1] << 8) | \
				(pData8)[0]; \
		} while (0)

uint32_t bct_priv_get_size(void)
{
	return BCT_LENGTH;
}

status_t
bct_priv_getdata_size(
	bct_handle hbct,
	bct_data_type data_type,
	uint32_t *psize,
	uint32_t *pnum_instances)
{
	uint32_t size = 0;
	uint32_t num_instances = 0;

	switch (data_type)
    {
		case bct_data_type_version:
		case bct_data_type_num_valid_sdramconfigs:
		case bct_data_type_odmoption:
		case bct_data_type_bootdevice_pagesizelog2:
		case bct_data_type_bootdevice_blocksizelog2:
		case bct_data_type_ramdump_carveout:
		case bct_data_type_num_enabled_bootloaders:
			size = sizeof(uint32_t);
			num_instances = 1;
			break;

		case bct_data_type_bootloader_attribute:
		case bct_data_type_bct_size:
		case bct_data_type_bootloader_startblock:
		case bct_data_type_bootloader_start_sector:
		case bct_data_type_bootloader_length:
		case bct_data_type_bootloader_loadaddress:
		case bct_data_type_bootloader_entrypoint:
		case bct_data_type_bootloader_version:
		case bct_data_type_bootfileset_start_sector:
		case bct_data_type_bootfileset_length:
		case bct_data_type_bootfileset_chksum:
		case bct_data_type_bootfileset_chksum_length:
			size = sizeof(uint32_t);
			num_instances = NVBOOT_MAX_BOOTLOADERS;
			break;

		case bct_data_type_sdram_configinfo:
			size = sizeof(NvBootSdramParams);
			num_instances = NVBOOT_BCT_MAX_SDRAM_SETS;
			break;
		case bct_data_type_rsakey_modulus:
			size = RSA_KEY_SIZE;
			num_instances = 1;
			break;

		case bct_data_type_rsa_psssig:
			size = RSA_KEY_SIZE;
			num_instances = 1;
			break;

		case bct_data_type_bootloader_cryptohash:
			size = sizeof(uint64_t) << 1;
			num_instances = NVBOOT_MAX_BOOTLOADERS;
			break;

		case bct_data_type_bootloader_rsa_psssig:
			size = RSA_KEY_SIZE;
			num_instances = NVBOOT_MAX_BOOTLOADERS;
			break;

		case bct_data_type_cryptohash:
			size = sizeof(uint64_t) << 1;
			num_instances = 1;
			break;

		case bct_data_type_signedsection:
			size = bct_priv_get_size() - bct_get_signdata_offset_priv();
			num_instances = 1;
			break;

		default:
			return ERR_NOT_SUPPORTED;
	}

	*psize = size;
	*pnum_instances = num_instances;

	return NO_ERROR;
}

status_t
bct_priv_get_data(
	bct_handle hbct,
	bct_data_type data_type,
	uint32_t *psize,
	uint32_t *pinstance,
	void *pdata)
{
	uint32_t size = 0;
	uint32_t instance = 0;
	uint32_t temp = 0;
	NvBootConfigTable *pbct = NULL;
	NvBootLoaderInfo *pbl_info = NULL;
    NvTegraBctAuxInternalData *pTbctAuxIntData = NULL;

	pbct = (NvBootConfigTable*)hbct;

	size = *psize;
	instance = *pinstance;

	if (instance < NVBOOT_MAX_BOOTLOADERS)
		pbl_info = &pbct->BootLoader[instance];

	switch (data_type)
	{
		case bct_data_type_version:
			memcpy(pdata, &pbct->BootDataVersion, size);
		break;
		case bct_data_type_bct_size:
		{
			temp = sizeof(NvBootConfigTable);
			memcpy(pdata, &temp, size);
			break;
		}

		case bct_data_type_num_enabled_bootloaders:
			memcpy(pdata, &(pbct->BootLoadersUsed), size);
			break;

		case bct_data_type_bootloader_attribute:
			memcpy(pdata, &(pbl_info->Attribute), size);
			break;

		case bct_data_type_bootloader_startblock:
			memcpy(pdata, &(pbl_info->StartBlock), size);
            break;

		case bct_data_type_bootloader_start_sector:
			memcpy(pdata, &(pbl_info->StartPage), size);
			break;

		case bct_data_type_bootloader_length:
			memcpy(pdata, &(pbl_info->Length), size);
            break;
		case bct_data_type_bootloader_version:
			memcpy(pdata, &(pbl_info->Version), size);
			break;

		case bct_data_type_bootloader_loadaddress:
			memcpy(pdata, &(pbl_info->LoadAddress), size);
            break;

		case bct_data_type_bootloader_entrypoint:
			memcpy(pdata,
					&(pbl_info->EntryPoint), size);
			break;

		case bct_data_type_bootloader_cryptohash:
			memcpy(pdata,
					&(pbl_info->Signature.CryptoHash.hash), size);
			break;

		case bct_data_type_bootloader_rsa_psssig:
			memcpy(pdata,
					&(pbl_info->Signature.RsaPssSig.Signature), size);
			break;

		case bct_data_type_bootdevice_blocksizelog2:
			memcpy(pdata,
					&(pbct->BlockSizeLog2), size);
			break;

		case bct_data_type_bootdevice_pagesizelog2:
			memcpy(pdata,
					&(pbct->PageSizeLog2), size);
			break;

		case bct_data_type_cryptohash:
			memcpy(pdata,
				&(pbct->Signature.CryptoHash.hash), size);
			break;

		case bct_data_type_rsa_psssig:
			memcpy(pdata,
				&(pbct->Signature.RsaPssSig.Signature), size);
			break;

		case bct_data_type_ramdump_carveout:
            pTbctAuxIntData = (NvTegraBctAuxInternalData *) (pbct->CustomerData +
                    NVBOOT_BCT_CUSTOMER_DATA_SIZE - sizeof(NvTegraBctAuxInternalData));
			memcpy(pdata,
				&(pTbctAuxIntData->NVDumperAddress), size);
			break;

		case bct_data_type_bootfileset_start_sector:
			pTbctAuxIntData = (NvTegraBctAuxInternalData *)
					(pbct->CustomerData +
					NVBOOT_BCT_CUSTOMER_DATA_SIZE -
					sizeof(NvTegraBctAuxInternalData));
			memcpy(pdata,
				&pTbctAuxIntData->BFSStartSector[instance],
				size);
			break;

		case bct_data_type_bootfileset_length:
			pTbctAuxIntData = (NvTegraBctAuxInternalData *)
					(pbct->CustomerData +
					NVBOOT_BCT_CUSTOMER_DATA_SIZE -
					sizeof(NvTegraBctAuxInternalData));
			memcpy(pdata,
				&pTbctAuxIntData->BFSLength[instance],
				size);
			break;

		case bct_data_type_bootfileset_chksum:
			pTbctAuxIntData = (NvTegraBctAuxInternalData *)
					(pbct->CustomerData +
					NVBOOT_BCT_CUSTOMER_DATA_SIZE -
					sizeof(NvTegraBctAuxInternalData));
			memcpy(pdata,
				&pTbctAuxIntData->BFSChksum[instance],
				size);
			break;

		case bct_data_type_bootfileset_chksum_length:
			pTbctAuxIntData = (NvTegraBctAuxInternalData *)
					(pbct->CustomerData +
					NVBOOT_BCT_CUSTOMER_DATA_SIZE -
					sizeof(NvTegraBctAuxInternalData));
			memcpy(pdata,
				&pTbctAuxIntData->BFSChksumLength[instance],
				size);
			break;

		case bct_data_type_rsakey_modulus:
			memcpy(pdata, &(pbct->Key.Modulus), size);
			break;

		default:
			return ERR_NOT_SUPPORTED;
	}
	return NO_ERROR;
}

status_t
bct_priv_set_data(
	bct_handle hbct,
	bct_data_type data_type,
	uint32_t *psize,
	uint32_t *pinstance,
	void *pdata)
{
	uint32_t size = 0;
	uint32_t instance = 0;
	NvBootConfigTable *pbct = NULL;
    NvTegraBctAuxInternalData *pTbctAuxIntData = NULL;

	instance = *pinstance;
	size = *psize;
	uint8_t *data8 = (uint8_t *)pdata;

	pbct = (NvBootConfigTable*)hbct;

	switch (data_type)
	{
		case bct_data_type_version:
			memcpy(&pbct->BootDataVersion, pdata, size);
		break;

		case bct_data_type_bootloader_attribute:
			memcpy(&(pbct->BootLoader[instance].Attribute), pdata, size);
		break;

		case bct_data_type_bootloader_length:
			memcpy(&(pbct->BootLoader[instance].Length), pdata, size);
		break;

		case bct_data_type_bootloader_version:
			memcpy(&(pbct->BootLoader[instance].Version), pdata, size);
            break;
		case bct_data_type_cryptohash:
		{
			uint32_t i;
			for (i=0; i < NVBOOT_CMAC_AES_HASH_LENGTH; i++)
				PACK_uint32(pbct->Signature.CryptoHash.hash[i],
					&data8[i * sizeof(uint32_t)]);
		}
		break;

		case bct_data_type_rsa_psssig:
		{
			uint32_t i;
			for (i=0; i < NVBOOT_SE_RSA_MODULUS_LENGTH_BITS / 8 / 4; i++)
				PACK_uint32(pbct->Signature.RsaPssSig.Signature[i],
					&data8[i * sizeof(uint32_t)]);
		}
		break;

		case bct_data_type_signedsection:
			memcpy(&(pbct->RandomAesBlock), pdata, size);
		break;

		case bct_data_type_bootloader_cryptohash:
		{
            uint32_t i;
			for (i=0; i < NVBOOT_CMAC_AES_HASH_LENGTH; i++)
				PACK_uint32(
					pbct->BootLoader[instance].Signature.CryptoHash.hash[i],
						&data8[i * sizeof(uint32_t)]);
		}
		break;

		case bct_data_type_ramdump_carveout:
            pTbctAuxIntData = (NvTegraBctAuxInternalData *) (pbct->CustomerData +
                    NVBOOT_BCT_CUSTOMER_DATA_SIZE - sizeof(NvTegraBctAuxInternalData));
		    memcpy(&(pTbctAuxIntData->NVDumperAddress),
									pdata, size);
		break;

		case bct_data_type_odmoption:
	        pTbctAuxIntData = (NvTegraBctAuxInternalData *) (pbct->CustomerData +
                    NVBOOT_BCT_CUSTOMER_DATA_SIZE - sizeof(NvTegraBctAuxInternalData));
			/* Update both instances */
		    memcpy(&(pTbctAuxIntData->CustomerOption),
									pdata, size);
		    memcpy(&(pTbctAuxIntData->CustomerOption2),
									pdata, size);
		break;

		default:
			return ERR_NOT_SUPPORTED;
	}

	return NO_ERROR;
}

uint32_t bct_get_signdata_offset_priv(void)
{
	return offsetof(NvBootConfigTable, RandomAesBlock);
}

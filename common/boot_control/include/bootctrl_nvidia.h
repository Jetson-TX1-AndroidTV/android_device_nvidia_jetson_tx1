/*
 * Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */

#ifndef _BOOTCTRL_NVIDIA_H_
#define _BOOTCTRL_NVIDIA_H_

#include <stdint.h>

#define OFFSET_SLOT_METADATA 0

#define BOOTCTRL_MAGIC 0x43424E00 /*magic number: '\0NBC' */
#define BOOTCTRL_SUFFIX_A           "-A"
#define BOOTCTRL_SUFFIX_B           "-B"
#define MAX_SLOTS 2
#define BOOTCTRL_VERSION 1
#define MAX_COUNT   7

/*This is just for test. Will define new slot_metadata partition */
#define BOOTCTRL_SLOTMETADATA_FILE  "/dev/block/platform/sdhci-tegra.3/by-name/SMD"

typedef struct slot_info {
    /*
     * boot priority of slot.
     * range [0:15]
     * 15 meaning highest priortiy,
     * 0 mean that the slot is unbootable.
     */
    uint8_t priority;
    /*
     * suffix of slots.
     */
    char suffix[2];
    /*
     * retry count of booting
     * range [0:7]
     */
    uint8_t retry_count;

    /* set true if slot can boot successfully */
    uint8_t boot_successful;

} slot_info_t;

typedef struct smd_partition {
    /* Magic number  for idetification */
    uint32_t magic;
    uint16_t version;
    uint16_t num_slots;
    /*slot parameter structure */
    slot_info_t slot_info[MAX_SLOTS];
} smd_partition_t;
#endif /* _BOOTCTRL_NVIDIA_H_ */

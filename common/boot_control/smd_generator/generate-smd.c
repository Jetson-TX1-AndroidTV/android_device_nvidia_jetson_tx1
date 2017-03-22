/*
 * Copyright (c) 2016, NVIDIA CORPORATION, All rights reserved.
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define OFFSET_SLOT_METADATA 0

#define BOOTCTRL_MAGIC 0x43424E00 /*magic number: '\0NBC' */
#define BOOTCTRL_SUFFIX_A           "-A"
#define BOOTCTRL_SUFFIX_B           "-B"
#define MAX_SLOTS 2
#define BOOTCTRL_VERSION 1

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

int main(int argc, char *argv[])
{
    smd_partition_t bootC;

    if (argc < 2) {
        printf("Usage: nv_smd_generator <out_file>\n");
        return -1;
    }

    bootC.slot_info[0].priority = 15;
    bootC.slot_info[0].retry_count = 7;
    bootC.slot_info[0].boot_successful = 1;
    strncpy(bootC.slot_info[0].suffix, BOOTCTRL_SUFFIX_A, 2);

    bootC.slot_info[1].priority = 0;
    bootC.slot_info[1].retry_count = 0;
    bootC.slot_info[1].boot_successful = 0;
    strncpy(bootC.slot_info[1].suffix, BOOTCTRL_SUFFIX_B, 2);

    bootC.magic = BOOTCTRL_MAGIC;
    bootC.version = BOOTCTRL_VERSION;
    bootC.num_slots = MAX_SLOTS;

    FILE* fout = fopen(argv[1], "w+");
    fwrite(&bootC, sizeof(smd_partition_t), 1, fout);
    fclose(fout);
    return 0;
}

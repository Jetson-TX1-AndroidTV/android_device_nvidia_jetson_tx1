/*
 * Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hardware/hardware.h>
#include "boot_control.h"
#include <cutils/properties.h>

#include "bootctrl_nvidia.h"

static int bootctrl_access_metadata(smd_partition_t *smd_partition, int writed)
{
    int fd;
    ssize_t sz, size = sizeof(smd_partition_t);
    char *buf = (char*)smd_partition;

    if((fd = open(BOOTCTRL_SLOTMETADATA_FILE, O_RDWR)) < 0) {
        printf("Fail to open metadata file\n");
        return -EIO;
    }
    if (lseek(fd, OFFSET_SLOT_METADATA, SEEK_SET) < 0) {
        printf("Error seeking to metadata offset\n");
        goto error;
    }

    /* Read/Write slot_medata */
    (writed)?(sz = write(fd, buf, size)):(sz = read(fd, buf, size));

    if(sz < 0) {

        printf("Fail to %s slot metadata \n", (writed)?("write"):("read"));
        goto error;
    }

    /* Check if the data is correct */
    if (smd_partition->magic != BOOTCTRL_MAGIC) {
        printf("Slot metadata is corrupted.\n");
        goto error;
    }

    return 0;

error:
    close(fd);
    return -EIO;
}

/*
 *Boot-loader pass the current slot value to kernel.
 *Read that value to identify which slot we use for now.
 */
static int bootctrl_get_active_slot()
{
    int i, err, slot;
    char prop[PROPERTY_VALUE_MAX];
    smd_partition_t smd_partition;

    err = bootctrl_access_metadata(&smd_partition, 0);
    if (err <0 )
       return  err;

    property_get("ro.boot.slot_suffix", prop, "");

    for (i = 0 ; i < MAX_SLOTS; i++) {
        if ((strncmp(prop, smd_partition.slot_info[i].suffix, 2)) == 0)
            return i;
    }

    printf("Can not get current slot ID!\n");
    return -EINVAL;
}

int bootctrl_mark_boot_successful(boot_control_module_t *module __unused)
{
    int err, slot;
    smd_partition_t smd_partition;

    err = bootctrl_access_metadata(&smd_partition, 0);
    if (err < 0)
       return  err;

    slot = bootctrl_get_active_slot();

    smd_partition.slot_info[slot].boot_successful = 1;
    smd_partition.slot_info[slot].retry_count = MAX_COUNT;

    return bootctrl_access_metadata(&smd_partition, 1);
}

int bootctrl_set_active_slot(boot_control_module_t *module __unused,
    unsigned slot)
{
    int err, slot_s;
    smd_partition_t smd_partition;

    if (slot >= MAX_SLOTS)
        return -EINVAL;

    err = bootctrl_access_metadata(&smd_partition, 0);
    if (err < 0)
        return  err;

    /*
     * Set the target slot priority to max value 15.
     * and reset the retry count to 7.
     */
    smd_partition.slot_info[slot].priority = 15;
    smd_partition.slot_info[slot].boot_successful = 0;
    smd_partition.slot_info[slot].retry_count = MAX_COUNT;

    slot?(slot_s = 0):(slot_s = 1);

    /*
     * Since we use target slot to boot,
     * lower source slot priority.
     */
    smd_partition.slot_info[slot_s].priority = 14;

    return bootctrl_access_metadata(&smd_partition, 1);
}

int bootctrl_set_slot_as_unbootable(boot_control_module_t *module __unused,
    unsigned slot)
{
    int err;
    smd_partition_t smd_partition;

    err = bootctrl_access_metadata(&smd_partition, 0);
    if (err < 0)
        return err;
    /*
     * As this slot is unbootable, set all of value to zero
     * so boot-loader does not rollback to this slot.
     */
    smd_partition.slot_info[slot].priority = 0;
    smd_partition.slot_info[slot].boot_successful = 0;
    smd_partition.slot_info[slot].retry_count = 0;

    return bootctrl_access_metadata(&smd_partition, 1);

}

int bootctrl_is_slot_bootable(boot_control_module_t *module __unused,
    unsigned slot)
{
    int err;
    smd_partition_t smd_partition;

    if (slot >= MAX_SLOTS)
        return -EINVAL;

    err = bootctrl_access_metadata(&smd_partition, 0);
    if (err < 0)
        return err;

    return (smd_partition.slot_info[slot].priority != 0);
}

const char *bootctrl_get_suffix(boot_control_module_t *module __unused,
    unsigned slot)
{
    const char *slots[MAX_SLOTS] = {
            BOOTCTRL_SUFFIX_A,
            BOOTCTRL_SUFFIX_B,
    };

    if (slot >= MAX_SLOTS)
        return NULL;

    return slots[slot];
}

int bootctrl_is_slot_marked_successful(boot_control_module_t *module __unused,
    unsigned slot)
{
    int err;
    smd_partition_t smd_partition;

    if (slot >= MAX_SLOTS)
        return -EINVAL;

    err = bootctrl_access_metadata(&smd_partition, 0);
    if (err < 0)
        return  err;

    return smd_partition.slot_info[slot].boot_successful;
}

unsigned bootctrl_get_current_slot(boot_control_module_t *module __unused)
{
    return bootctrl_get_active_slot();
}

void bootctrl_dump_slot_info(boot_control_module_t *module __unused)
{
    int err, i;
    smd_partition_t smd_partition;

    err = bootctrl_access_metadata(&smd_partition, 0);
    if (err < 0)
        return;

    printf("magic:0x%x, \
            version: %d \
            num_slots: %d\n",
            smd_partition.magic,
            smd_partition.version,
            smd_partition.num_slots);

    for (i = 0; i < MAX_SLOTS; i++) {
        printf("slot: %d, \
            priority: %d, \
            suffix: %.2s, \
            retry_count: %d, \
            boot_successful: %d\n",
            i,
            smd_partition.slot_info[i].priority,
            smd_partition.slot_info[i].suffix,
            smd_partition.slot_info[i].retry_count,
            smd_partition.slot_info[i].boot_successful);
    }

}

unsigned bootctrl_get_number_slots(boot_control_module_t *module __unused)
{
    int err;
    smd_partition_t smd_partition;

    err = bootctrl_access_metadata(&smd_partition, 0);
    if (err < 0)
        return  err;

    return smd_partition.num_slots;
}

void bootctrl_init(boot_control_module_t *module __unused)
{
    smd_partition_t smd_partition;

    bootctrl_access_metadata(&smd_partition, 0);

    if (smd_partition.magic == BOOTCTRL_MAGIC)
        return;

    /*
     * Since SMD partition is empty,
     * we set initial value for that.
     *
     */
    printf("Slot metadata is not initial\n");

    smd_partition.magic = BOOTCTRL_MAGIC;

    smd_partition.num_slots = MAX_SLOTS;
    smd_partition.version = BOOTCTRL_VERSION;
    smd_partition.slot_info[0].priority = 14;
    memcpy(smd_partition.slot_info[0].suffix, BOOTCTRL_SUFFIX_A, 2);
    smd_partition.slot_info[0].boot_successful = 1;
    smd_partition.slot_info[0].retry_count = MAX_COUNT;

    smd_partition.slot_info[1].priority = 0;
    memcpy(smd_partition.slot_info[1].suffix, BOOTCTRL_SUFFIX_B, 2);
    smd_partition.slot_info[1].boot_successful = 0;
    smd_partition.slot_info[1].retry_count = 0;

    bootctrl_access_metadata(&smd_partition, 1);
}

boot_control_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                 = HARDWARE_MODULE_TAG,
        .module_api_version  = BOOT_CONTROL_MODULE_API_VERSION_0_1,
        .hal_api_version     = HARDWARE_HAL_API_VERSION,
        .id                  = BOOT_CONTROL_HARDWARE_MODULE_ID,
        .name                = "Nvidia Boot Control HAL",
        .author              = "Nvidia Corporation",
    },
    .init                   = bootctrl_init,
    .getNumberSlots         = bootctrl_get_number_slots,
    .getCurrentSlot         = bootctrl_get_current_slot,
    .markBootSuccessful     = bootctrl_mark_boot_successful,
    .setActiveBootSlot      = bootctrl_set_active_slot,
    .setSlotAsUnbootable    = bootctrl_set_slot_as_unbootable,
    .isSlotBootable         = bootctrl_is_slot_bootable,
    .getSuffix              = bootctrl_get_suffix,
    .isSlotMarkedSuccessful = bootctrl_is_slot_marked_successful,
    .dumpSlotInfo           = bootctrl_dump_slot_info,
};

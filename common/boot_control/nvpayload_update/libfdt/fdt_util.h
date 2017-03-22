/*
 * Copyright (c) 2016 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software and related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef _FDT_UTIL_H
#define _FDT_UTIL_H

#define BL_DTB_PATH "/dev/block/platform/sdhci-tegra.3/by-name/RP1"
#define KERNEL_DTB_PATH "/dev/block/platform/sdhci-tegra.3/by-name/DTB"

#define BOOTCTRL_SUFFIX_A           "-A"
#define BOOTCTRL_SUFFIX_B           "-B"
#define FNAME_LEN   256

typedef enum {
    DTB_NO_ERROR = 0,
    DTB_ERR_NOMEMORY,
    DTB_ERR_NOTFUND
} dtb_error;

enum dtb_type {
    BL_DTB = 0,
    KERNEL_DTB
};

dtb_error is_dtb_valid(void *new_fdt, enum dtb_type type, int slot);

#endif // _FDT_UTIL_H

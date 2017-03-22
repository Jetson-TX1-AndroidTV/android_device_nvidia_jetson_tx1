/*
 * Copyright (c) 2016 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software and related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libfdt.h>
#include "fdt_util.h"

static const char* get_basename(const char* path) {
    char* basename = strrchr(path, '/');

    if (basename == NULL)
        return path;
    else
        return basename + 1; /* strip '/' */
}

bool dt_get_prop_string(const void *fdt, int node_offset, char *prop_name,
                                                    const char **res) {
    if (!fdt || !prop_name || !res)
        return false;

    *res = fdt_getprop(fdt, node_offset, prop_name, NULL);
    if (!*res)
        return false;
     return true;
}

static char kernel_dtb[FNAME_LEN];
static char bl_dtb[FNAME_LEN];

static dtb_error get_current_dtb(enum dtb_type type, int slot) {
    long size;
    FILE *fd;
    char path[FNAME_LEN];
    dtb_error status = DTB_NO_ERROR;
    void *cur_fdt;
    const char *cur_fname;

    switch (type) {
    case KERNEL_DTB:
        if (slot)
            sprintf(path, "%s%s", KERNEL_DTB_PATH, BOOTCTRL_SUFFIX_A);
        else
            sprintf(path, "%s%s", KERNEL_DTB_PATH, BOOTCTRL_SUFFIX_B);

        break;
    case BL_DTB:
        if (slot)
            sprintf(path, "%s%s", BL_DTB_PATH, BOOTCTRL_SUFFIX_A);
        else
            sprintf(path, "%s%s", BL_DTB_PATH, BOOTCTRL_SUFFIX_B);

        break;
    }

    fd = fopen(path, "r");
    fseek(fd, 0L, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0L, SEEK_SET);

    cur_fdt = malloc(size);
    if(!cur_fdt) {
        status = DTB_ERR_NOMEMORY;
        goto exit;
    }

    fread(cur_fdt, 1,  size, fd);

    if (!dt_get_prop_string(cur_fdt, 0, "nvidia,dtsfilename", &cur_fname)) {
        status = DTB_ERR_NOTFUND;
        goto exit;
    }

    cur_fname = get_basename(cur_fname);
    if (type == KERNEL_DTB)
        strncpy(kernel_dtb, cur_fname, strlen(cur_fname));
    else if (type == BL_DTB)
        strncpy(bl_dtb, cur_fname, strlen(cur_fname));

exit:
    free(cur_fdt);
    fclose(fd);
    return status;
}



dtb_error is_dtb_valid(void *new_fdt, enum dtb_type type, int slot) {
    int32_t ret;
    const char *new_fname;
    dtb_error status = DTB_NO_ERROR;

    /*
     * 1) call check_header for local copy, if success
     * 2) if header is valid, Read node value from RAM dtb and read the same
     *    value from the file.
     * 3) compare both, if it is same, return true.
     */

    /* check new dtb header */
    ret = fdt_check_header(new_fdt);
    if (ret != 0) {
        return DTB_ERR_NOTFUND;
    }

    if (!*bl_dtb) {
        status = get_current_dtb(BL_DTB, slot);
        if (status)
            return status;
    }

    if (!*kernel_dtb) {
        status = get_current_dtb(KERNEL_DTB, slot);
        if (status)
            return status;
    }

    if (!dt_get_prop_string(new_fdt, 0, "nvidia,dtsfilename", &new_fname)) {
        return DTB_ERR_NOTFUND;
    }

    new_fname = get_basename(new_fname);

    /* Names are literal strings, hence using string APIs such as strcmp */
    if (type == KERNEL_DTB) {
        if (strcmp(new_fname, kernel_dtb) != 0)
            return DTB_ERR_NOTFUND;
    } else if (type == BL_DTB) {
        if (strcmp(new_fname, kernel_dtb) != 0)
            return DTB_ERR_NOTFUND;
    }

    return DTB_NO_ERROR;
}

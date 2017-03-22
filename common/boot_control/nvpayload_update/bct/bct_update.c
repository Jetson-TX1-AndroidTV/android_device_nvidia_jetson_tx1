/*
 * Copyright (c) 2016 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software and related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include <err.h>
#include "bct.h"
#include "bct_private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <sys/types.h>

int update_bct_signedsection(uint8_t *curbct, uint8_t *newbct, unsigned int size)
{
    status_t status = NO_ERROR;
    bct_handle cur_bct = NULL;
    bct_handle new_bct = NULL;
    bct_data_type sigtype;
    uint32_t bctsize;
    uint8_t  *bctptr = NULL;
    uint8_t  *sigsec = NULL;
    uint32_t sigsec_len = 0;
    uint32_t sigsec_instances = 0;
    uint8_t  *sig = NULL;
    uint32_t sig_len = 0;
    uint32_t sig_instances = 0;

   /* Initialize bct from blob */
    status = bct_init(&size, newbct, &new_bct);
    if (status != NO_ERROR)
        goto end_updatebct;

    /* Initialize bct of device */
    bctsize = get_bct_size();

    bctptr = curbct;

    status = bct_init(&bctsize, bctptr, &cur_bct);
    if (status != NO_ERROR)
        goto end_updatebct;

    /* copy signature of signed_section from new bct to old bct */
    sigtype = bct_data_type_cryptohash;

    status = bct_get_data(new_bct, sigtype, &sig_len, &sig_instances, NULL);
    if (status != NO_ERROR)
        goto end_updatebct;
    sig = (uint8_t* )malloc(sig_len);
    status = bct_get_data(new_bct, sigtype, &sig_len, &sig_instances, sig);
    if (status != NO_ERROR)
        goto end_updatebct;
    status = bct_set_data(cur_bct, sigtype, &sig_len, &sig_instances, sig);
    if (status != NO_ERROR)
        goto end_updatebct;

    /* copy signed section from new bct to old bct */
    status = bct_get_data(new_bct, bct_data_type_signedsection, &sigsec_len,
                    &sigsec_instances, NULL);
    if (status != NO_ERROR)
        goto end_updatebct;

    sigsec = newbct + bct_get_signdata_offset();
    status = bct_set_data(cur_bct, bct_data_type_signedsection, &sigsec_len,
                    &sigsec_instances, sigsec);
    if (status != NO_ERROR)
        goto end_updatebct;

    memcpy(newbct, bctptr, size);

    return 0;

end_updatebct:
    return status;
}

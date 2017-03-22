/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (c) 2012-2014, NVIDIA CORPORATION.  All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "powerhal.h"

static struct powerhal_info *pInfo;
static struct input_dev_map input_devs[] = {
        {-1, "raydium_ts\n"},
        {-1, "touch\n"},
        {-1, "touch_fusion\n"}
       };

/*
 * The order in camera_cap array should match with
 * use case order in camera_usecase_t.
 * if min_online_cpus or max_online_cpus is zero, then
 * it won't be applied.
 * Freq is in KHz.
 * remove the Cap for Ardbeg
 */
static camera_cap_t camera_cap[] = {
    /* still preview
     * {min_online_cpus, max_online_cpus, freq, minFreq,
     *  minCpuHint, maxCpuHint, minGpuHint, maxGpuHint, fpsHint}
    */
    {0, 0, 0, 0, 0, 0, 0, 0, 30},
    /* video preview
     * {min_online_cpus, max_online_cpus, freq, minFreq,
     *  minCpuHint, maxCpuHint, minGpuHint, maxGpuHint, fpsHint}
    */
    {0, 0, 0, 0, 0, 0, 0, 0, 30},
    /* video record
     * {min_online_cpus, max_online_cpus, freq, minFreq,
     *  minCpuHint, maxCpuHint, minGpuHint, maxGpuHint, fpsHint}
    */
    {0, 0, 0, 0, 0, 0, 0, 0, 30},
    /* high fps video record
     * {min_online_cpus, max_online_cpus, freq, minFreq,
     *  minCpuHint, maxCpuHint, minGpuHint, maxGpuHint, fpsHint}
    */
    // Below mentioned values are based on experiments done to achieve
    // high fps.
    {4, 0, 0, 1428000, 0, 0, 0, 0, 120},
};

static void ardbeg_power_init(struct power_module *module)
{
    if (!pInfo)
        pInfo = (powerhal_info*)malloc(sizeof(powerhal_info));
    pInfo->input_devs = input_devs;
    pInfo->input_cnt = sizeof(input_devs)/sizeof(struct input_dev_map);

    common_power_init(module, pInfo);
}

static void ardbeg_power_set_interactive(struct power_module *module, int on)
{
    common_power_set_interactive(module, pInfo, on);
}

static void ardbeg_power_hint(struct power_module *module, power_hint_t hint,
                            void *data)
{
    common_power_hint(module, pInfo, hint, data);
}

static int ardbeg_power_open(const hw_module_t *module, const char *name,
                            hw_device_t **device)
{
    if (strcmp(name, POWER_HARDWARE_MODULE_ID))
        return -EINVAL;

    if (!pInfo) {
        pInfo = (powerhal_info*)calloc(1, sizeof(powerhal_info));

        common_power_open(pInfo);
        common_power_camera_init(pInfo, camera_cap);
    }

    return 0;
}

static struct hw_module_methods_t power_module_methods = {
    .open = ardbeg_power_open,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_2,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "Ardbeg Power HAL",
        .author = "NVIDIA",
        .methods = &power_module_methods,
        .dso = NULL,
        .reserved = {0},
    },

    .init = ardbeg_power_init,
    .setInteractive = ardbeg_power_set_interactive,
    .powerHint = ardbeg_power_hint,
};

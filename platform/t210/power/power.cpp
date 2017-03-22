/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (c) 2014, NVIDIA CORPORATION.  All rights reserved.
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
#include <pthread.h>
#include <sys/prctl.h>
#ifdef ENABLE_SATA_STANDBY_MODE
#include "tegra_sata_hal.h"
#endif

#define FOSTER_E_HDD    "/dev/block/sda"
#define HDD_STANDBY_TIMEOUT     60
#define CPU_CC_STATE_NODE "/sys/kernel/debug/cpuidle_t210/fast_cluster_states_enable"
#define CPU_CC_IDLE 0x1
#define CPU_CC_ON 0xcf
#define CPU_CEILING_NODE "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define CPU_CEILING_IDLE 1836000
#define CPU_CEILING_ON 2014500
#define CPU_FLOOR_ON 1132800
#define CPU_FLOOR_IDLE 1734000
#define CPU_FLOOR_NODE "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq"
#define ETHERNET_POWER_SAVER_NODE "/sys/kernel/rt8168_power/mode"
#define GPU_DELAY_MAX 20
#define GPU_FLOOR_ON 768000000
#define GPU_FLOOR_IDLE 844800000
#define GPU_MIN_FREQ "/sys/devices/platform/host1x/gpu.0/devfreq/gpu.0/min_freq"
#define GPU_PMU_STATE "/sys/devices/platform/gpu.0/pmu_state"
#define GPU_RAIL_GATE_NODE "/sys/devices/platform/host1x/gpu.0/railgate_enable"
#define GPU_STATE "/dev/nvhost-gpu"
#define GPU_AELPG_NODE "/sys/devices/platform/gpu.0/aelpg_enable"
#define GPU_BLCG_NODE "/sys/devices/platform/gpu.0/blcg_enable"
#define GPU_ELCG_NODE "/sys/devices/platform/gpu.0/elcg_enable"
#define GPU_ELPG_NODE "/sys/devices/platform/gpu.0/elpg_enable"
#define GPU_SLCG_NODE "/sys/devices/platform/gpu.0/slcg_enable"
#define SOC_DISABLE_DVFS_NODE "/sys/module/tegra21_dvfs/parameters/disable_core"
#define WIFI_PM_NODE "/sys/class/net/wlan0/device/rf_test/pm"
#define WIFI_PM_ENABLE "pm_enable"
#define WIFI_PM_DISABLE "pm_disable"
#define POWER_LEVEL_FLOOR_PROP "persist.sys.power.powerlevel"

static struct powerhal_info *pInfo;
static struct input_dev_map input_devs[] = {
        {-1, "touch\n"},
       };

/*
 * The order in camera_cap array should match with
 * use case order in camera_usecase_t.
 * if min_online_cpus or max_online_cpus is zero, then
 * it won't be applied.
 * Freq is in KHz.
 */
static camera_cap_t camera_cap[] = {
    /* still preview
     * {min_online_cpus, max_online_cpus, freq, minFreq,
     *  minCpuHint, maxCpuHint, minGpuHint, maxGpuHint, fpsHint}
    */
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* video preview
     * {min_online_cpus, max_online_cpus, freq, minFreq,
     *  minCpuHint, maxCpuHint, minGpuHint, maxGpuHint, fpsHint}
    */
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* video record
     * {min_online_cpus, max_online_cpus, freq, minFreq,
     *  minCpuHint, maxCpuHint, minGpuHint, maxGpuHint, fpsHint}
    */
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* high fps video record
     * {min_online_cpus, max_online_cpus, freq, minFreq,
     *  minCpuHint, maxCpuHint, minGpuHint, maxGpuHint, fpsHint}
    */
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
};

static int booting;

static void* set_gpu_power_knobs_off(void*)
{
    char buf[4];

    if (booting)
        for (int cnt = 0; cnt < GPU_DELAY_MAX &&
                !sysfs_exists(GPU_STATE); ++cnt)
            sleep(1);

    sleep(5);
    sysfs_write_int(GPU_RAIL_GATE_NODE, 0);

    for (int cnt = 0; cnt < GPU_DELAY_MAX; ++cnt) {
        sleep(1);
        memset(buf, 0, sizeof(buf));
        sysfs_read(GPU_PMU_STATE, buf, sizeof(buf));

        if (atol(buf))
            cnt = GPU_DELAY_MAX + 1;
    }

    sleep(1);
    sysfs_write_int(GPU_ELPG_NODE, 0);
    sysfs_write_int(GPU_SLCG_NODE, 0);
    sysfs_write_int(GPU_BLCG_NODE, 0);
    sysfs_write_int(GPU_ELCG_NODE, 0);
    booting = 0;
    ALOGI("PowerHal: gpu power knobs are off");

    return NULL;
}


static void set_power_level_floor(int on)
{
    char platform[PROPERTY_VALUE_MAX+1];
    char buf[4] = {0};

    if (!(get_property_bool(POWER_LEVEL_FLOOR_PROP, true)))
        return;

    property_get("ro.hardware", platform, "");

    if (strncmp(platform,"darcy", 5))
        return;

    if (on) {
        sysfs_write_int(CPU_CC_STATE_NODE, CPU_CC_ON);
        sysfs_write_int(CPU_FLOOR_NODE, CPU_FLOOR_ON);
        sysfs_write_int(CPU_CEILING_NODE, CPU_CEILING_ON);
        sysfs_write_int(GPU_MIN_FREQ, GPU_FLOOR_ON);
    } else {
        sysfs_write_int(CPU_CC_STATE_NODE, CPU_CC_IDLE);
        sysfs_write_int(CPU_FLOOR_NODE, CPU_FLOOR_IDLE);
        sysfs_write_int(CPU_CEILING_NODE, CPU_CEILING_IDLE);
        sysfs_write_int(GPU_MIN_FREQ, GPU_FLOOR_IDLE);
    }

    sysfs_write_int(ETHERNET_POWER_SAVER_NODE, 0);
    sysfs_write_int(SOC_DISABLE_DVFS_NODE, 1);
    ALOGI("PowerHal: Power policy has been applied");

    sysfs_read(GPU_ELPG_NODE, buf, sizeof(buf));

    if (!atol(buf)) {
        ALOGI("PowerHal: Power knobs are already set");
        return;
    }

    if (booting) {
        pthread_t gpuk_t;

        if (pthread_create(&gpuk_t, NULL, &set_gpu_power_knobs_off, NULL))
            ALOGE("PowerHal: Failed to create thread %s", __func__);
    } else
        set_gpu_power_knobs_off(NULL);
}

static void loki_e_power_init(struct power_module *module)
{
    if (!pInfo)
        pInfo = (powerhal_info*)malloc(sizeof(powerhal_info));
    pInfo->input_devs = input_devs;
    pInfo->input_cnt = sizeof(input_devs)/sizeof(struct input_dev_map);

    booting = 1;
    common_power_init(module, pInfo);
}

static void loki_e_power_set_interactive(struct power_module *module, int on)
{
    int error = 0;

    common_power_set_interactive(module, pInfo, on);
    set_power_level_floor(on);

#ifdef ENABLE_SATA_STANDBY_MODE
    if (!access(FOSTER_E_HDD, F_OK)) {
        /*
        * Turn-off Foster HDD at display off
        */
        ALOGI("HAL: Display is %s, set HDD to %s standby mode.", on?"on":"off", on?"disable":"enter");
        if (on) {
            error = hdd_disable_standby_timer();
            if (error)
                ALOGE("HAL: Failed to set standby timer, error: %d", error);
        }
        else {
            error = hdd_set_standby_timer(HDD_STANDBY_TIMEOUT);
            if (error)
                ALOGE("HAL: Failed to set standby timer, error: %d", error);
        }
    }
#endif
}

static void loki_e_power_hint(struct power_module *module, power_hint_t hint,
                            void *data)
{
    common_power_hint(module, pInfo, hint, data);
}

static int loki_e_power_open(const hw_module_t *module, const char *name,
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
    .open = loki_e_power_open,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_2,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "Loki-E Power HAL",
        .author = "NVIDIA",
        .methods = &power_module_methods,
        .dso = NULL,
        .reserved = {0},
    },

    .init = loki_e_power_init,
    .setInteractive = loki_e_power_set_interactive,
    .powerHint = loki_e_power_hint,
};

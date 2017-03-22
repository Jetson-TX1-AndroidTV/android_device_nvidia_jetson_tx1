/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (c) 2012-2015, NVIDIA CORPORATION.  All rights reserved.
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
#define LOG_TAG "powerHAL::common"

#include <hardware/hardware.h>
#include <hardware/power.h>

#include "powerhal_utils.h"
#include "powerhal.h"

#include "nvos.h"

//
// NOTE: BEWARE: These enumerations are duplicated from phs.h because phs.h
// isn't available in customer builds.
//
// TODO: Consider implementing a binary shim library that links to libphs.so
// and that encapsulates the phs.h definitions in compile time. Then link to
// the shim from nvpowerhal and use a different API between powerhal and the
// shim to ask it send various predetermined hints.
//
typedef enum
{
    NvUsecase_generic   = 0x00000001,
    NvUsecase_graphics  = 0x00000002,
    NvUsecase_camera    = 0x00000004,
    NvUsecase_video     = 0x00000008,
    NvUsecase_Force32   = 0x7fffffff
} NvUsecase;

typedef enum
{
    NvHintType_ThroughputHint   = 0,
    NvHintType_FramerateTarget  = 1,
    NvHintType_MinCPU           = 2,
    NvHintType_MaxCPU           = 3,
    NvHintType_MinGPU           = 4,
    NvHintType_MaxGPU           = 5,
    NvHintType_LastReserved     = 63,
    NvHintType_Force32          = 0x7FFFFFFF
} NvHintType;

#define NVHINT_DEFAULT_TAG 0x00000000U

#define PHS_DEBUG

#ifdef POWER_MODE_SET_INTERACTIVE
static int get_system_power_mode(void);
static void set_interactive_governor(int mode);

static const interactive_data_t interactive_data_array[NVCPL_HINT_COUNT+1] =
{
    { "1122000", "65 304000:75 1122000:80", "19000", "20000", "0", "41000", "90" },
    { "1020000", "65 256000:75 1020000:80", "19000", "20000", "0", "30000", "99" },
    { "640000", "65 256000:75 640000:80", "80000", "20000", "2", "30000", "99" },
    { "1020000", "65 256000:75 1020000:80", "19000", "20000", "0", "30000", "99" },
    { "420000", "80",                     "80000", "300000","2", "30000", "99" }
};
#endif

// CPU/EMC ratio table source sysfs
#define CPU_EMC_RATIO_SRC_NODE "/sys/kernel/tegra_cpu_emc/table_src"

static const int VSyncActiveBoostFreq = 300000;

static int get_input_count(void)
{
    int i = 0;
    int ret;
    char path[80];
    char name[32];

    while(1)
    {
        snprintf(path, sizeof(path), "/sys/class/input/input%d/name", i);
        ret = access(path, F_OK);
        if (ret < 0)
            break;
        memset(name, 0, 32);
        sysfs_read(path, name, 32);
        ALOGI("input device id:%d present with name:%s", i++, name);
    }
    return i;
}

static void find_input_device_ids(struct powerhal_info *pInfo)
{
    int i = 0;
    int status;
    int count = 0;
    char path[80];
    char name[MAX_CHARS];

    while (1)
    {
        snprintf(path, sizeof(path), "/sys/class/input/input%d/name", i);
        if (access(path, F_OK) < 0)
            break;
        else {
            memset(name, 0, MAX_CHARS);
            sysfs_read(path, name, MAX_CHARS);
            for (int j = 0; j < pInfo->input_cnt; j++) {
                status = (-1 == pInfo->input_devs[j].dev_id)
                    && (0 == strncmp(name,
                    pInfo->input_devs[j].dev_name, MAX_CHARS));
                if (status) {
                    ++count;
                    pInfo->input_devs[j].dev_id = i;
                    ALOGI("find_input_device_ids: %d %s",
                        pInfo->input_devs[j].dev_id,
                        pInfo->input_devs[j].dev_name);
                }
            }
            ++i;
        }

        if (count == pInfo->input_cnt)
            break;
    }
}

static int check_hint(struct powerhal_info *pInfo, power_hint_t hint, uint64_t *t)
{
    struct timespec ts;
    uint64_t time;

    if (hint >= MAX_POWER_HINT_COUNT) {
        ALOGE("Invalid power hint: 0x%x", hint);
        return -1;
    }

    clock_gettime(CLOCK_MONOTONIC, &ts);
    time = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;

    if (pInfo->hint_time[hint] && pInfo->hint_interval[hint] &&
        (time - pInfo->hint_time[hint] < pInfo->hint_interval[hint]))
        return -1;

    *t = time;

    return 0;
}

void common_power_camera_init(struct powerhal_info *pInfo, camera_cap_t *cap)
{
    char const* dlsym_error;
    int target;
    int i;

    if (!pInfo)
    {
        ALOGE("pInfo is NULL");
        return;
    }

    memset(&pInfo->camera_power, 0, sizeof(pInfo->camera_power));

    pInfo->camera_power.fd_gpu = -1;
    pInfo->camera_power.fd_cpu_freq_min = -1;
    pInfo->camera_power.fd_cpu_freq_max = -1;
    pInfo->camera_power.fd_min_online_cpus = -1;

    pInfo->camera_power.target_fps = 0;
    pInfo->camera_power.cam_cap = cap;
    pInfo->camera_power.usecase_index = -1;

    /* initalization primitives for regular hints thread */
    pthread_cond_init(&pInfo->wait_cond,NULL);
    pthread_mutex_init(&pInfo->wait_mutex,NULL);
    pInfo->regular_hints_thread = 0;
    pInfo->exit_hints_thread = false;

    return;

err_camera_init:
    if (pInfo->camera_power.handle)
        dlclose(pInfo->camera_power.handle);
}

static bool is_available_frequency(struct powerhal_info *pInfo, int freq)
{
    int i;

    for(i = 0; i < pInfo->num_available_frequencies; i++) {
        if(pInfo->available_frequencies[i] == freq)
            return true;
    }

    return false;
}

static void common_libphs_open (struct powerhal_info *pInfo)
{
    pInfo->libphs_handle = dlopen("libphs.so", RTLD_NOW);

    if (pInfo->libphs_handle) {
        pInfo->NvVaSendThroughputHints = (sendhints_fn_t)dlsym(pInfo->libphs_handle, "NvVaSendThroughputHints");
        pInfo->NvCancelThroughputHints = (cancelhints_fn_t)dlsym(pInfo->libphs_handle, "NvCancelThroughputHints");

        if (pInfo->NvVaSendThroughputHints && pInfo->NvCancelThroughputHints)
            return;
    }

    pInfo->NvVaSendThroughputHints = NULL;
    pInfo->NvCancelThroughputHints = NULL;
}

void common_power_open(struct powerhal_info *pInfo)
{
    int i;
    int size = 256;
    char *pch;

    if (0 == pInfo->input_devs || 0 == pInfo->input_cnt)
        pInfo->input_cnt = get_input_count();
    else
        find_input_device_ids(pInfo);

    // Initialize timeout poker
    Barrier readyToRun;
    pInfo->mTimeoutPoker = new TimeoutPoker(&readyToRun);
    readyToRun.wait();

    // Read available frequencies
    char *buf = (char*)malloc(sizeof(char) * size);
    memset(buf, 0, size);
    sysfs_read("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies",
               buf, size);

    // Determine number of available frequencies
    pch = strtok(buf, " ");
    pInfo->num_available_frequencies = -1;
    while(pch != NULL)
    {
        pch = strtok(NULL, " ");
        pInfo->num_available_frequencies++;
    }

    // Store available frequencies in a lookup array
    pInfo->available_frequencies = (int*)malloc(sizeof(int) * pInfo->num_available_frequencies);
    sysfs_read("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies",
               buf, size);
    pch = strtok(buf, " ");
    for(i = 0; i < pInfo->num_available_frequencies; i++)
    {
        pInfo->available_frequencies[i] = atoi(pch);
        pch = strtok(NULL, " ");
    }

    // Store LP cluster max frequency
    sysfs_read("/sys/devices/system/cpu/cpuquiet/tegra_cpuquiet/idle_top_freq",
                buf, size);
    pInfo->lp_max_frequency = atoi(buf);

    pInfo->interaction_boost_frequency = pInfo->lp_max_frequency;
    pInfo->animation_boost_frequency = pInfo->lp_max_frequency;

    for (i = 0; i < pInfo->num_available_frequencies; i++)
    {
        if (pInfo->available_frequencies[i] >= 1326000) {
            pInfo->interaction_boost_frequency = pInfo->available_frequencies[i];
            break;
        }
    }

    for (i = 0; i < pInfo->num_available_frequencies; i++)
    {
        if (pInfo->available_frequencies[i] >= 1024000) {
            pInfo->animation_boost_frequency = pInfo->available_frequencies[i];
            break;
        }
    }

    // Store CPU0 max frequency
    sysfs_read(SYS_NODE_CPU0_MAX_FREQ, buf, size);
    pInfo->cpu0_max_frequency = atoi(buf);

    // Initialize hint intervals in usec
    //
    // Set the interaction timeout to be slightly shorter than the duration of
    // the interaction boost so that we can maintain is constantly during
    // interaction.
    pInfo->hint_interval[POWER_HINT_VSYNC] = 0;
    pInfo->hint_interval[POWER_HINT_INTERACTION] = 90000;
    pInfo->hint_interval[POWER_HINT_APP_PROFILE] = 200000;
    pInfo->hint_interval[POWER_HINT_APP_LAUNCH] = 1500000;
    pInfo->hint_interval[POWER_HINT_SHIELD_STREAMING] = 500000;
    pInfo->hint_interval[POWER_HINT_HIGH_RES_VIDEO] = 500000;
    pInfo->hint_interval[POWER_HINT_VIDEO_DECODE] = 500000;
    pInfo->hint_interval[POWER_HINT_MIRACAST] = 500000;
    pInfo->hint_interval[POWER_HINT_AUDIO_SPEAKER] = 500000;
    pInfo->hint_interval[POWER_HINT_AUDIO_OTHER] = 500000;
    pInfo->hint_interval[POWER_HINT_AUDIO_LOW_LATENCY] = 500000;
    pInfo->hint_interval[POWER_HINT_DISPLAY_ROTATION] = 200000;
    pInfo->hint_interval[POWER_HINT_POWER_MODE] = 0;

    // Initialize AppProfile defaults
    pInfo->defaults.min_freq = 0;
    pInfo->defaults.max_freq = PM_QOS_DEFAULT_VALUE;
    pInfo->defaults.core_cap = PM_QOS_DEFAULT_VALUE;
    pInfo->defaults.gpu_cap = PM_QOS_DEFAULT_VALUE;
    pInfo->defaults.fan_cap = 70;
    pInfo->defaults.power_cap = 0;

    // Initialize fds
    pInfo->fds.app_min_freq = -1;
    pInfo->fds.app_max_freq = -1;
    pInfo->fds.app_max_online_cpus = -1;
    pInfo->fds.app_min_online_cpus = -1;
    pInfo->fds.app_max_gpu = -1;
    pInfo->fds.app_min_gpu = -1;
    pInfo->fds.vsync_min_cpu = -1;

    // Initialize features
    pInfo->features.fan = sysfs_exists("/sys/devices/platform/pwm-fan/pwm_cap");

    // Initialize libphs
    common_libphs_open(pInfo);

    free(buf);
}

static void set_vsync_min_cpu_freq(struct powerhal_info *pInfo, int enabled)
{
    if (enabled && pInfo->fds.vsync_min_cpu == -1) {
        pInfo->fds.vsync_min_cpu =
        pInfo->mTimeoutPoker->requestPmQos(PMQOS_CONSTRAINT_CPU_FREQ, PM_QOS_BOOST_PRIORITY, PM_QOS_DEFAULT_VALUE, VSyncActiveBoostFreq);
    } else if (!enabled && pInfo->fds.vsync_min_cpu >= 0) {
        close(pInfo->fds.vsync_min_cpu);
        pInfo->fds.vsync_min_cpu = -1;
    }

    ALOGV("%s: set min CPU floor =%i", __func__, VSyncActiveBoostFreq);
}

static void set_app_profile_min_cpu_freq(struct powerhal_info *pInfo, int value)
{
    if (value < 0)
        value = pInfo->defaults.min_freq;

    if (pInfo->fds.app_min_freq >= 0) {
        close(pInfo->fds.app_min_freq);
        pInfo->fds.app_min_freq = -1;
    }
    pInfo->fds.app_min_freq =
        pInfo->mTimeoutPoker->requestPmQos(PMQOS_CONSTRAINT_CPU_FREQ, PM_QOS_APP_PROFILE_PRIORITY, PM_QOS_DEFAULT_VALUE, value);

    ALOGV("%s: set min CPU floor =%d", __func__, value);
}

static void set_app_profile_max_cpu_freq(struct powerhal_info *pInfo, int value)
{
    if (value <= 0)
        value = pInfo->defaults.max_freq;

    if (pInfo->fds.app_max_freq >= 0) {
        close(pInfo->fds.app_max_freq);
        pInfo->fds.app_max_freq = -1;
    }
    pInfo->fds.app_max_freq =
        pInfo->mTimeoutPoker->requestPmQos(PMQOS_CONSTRAINT_CPU_FREQ, PM_QOS_APP_PROFILE_PRIORITY, value, PM_QOS_DEFAULT_VALUE);

    ALOGV("%s: set max CPU ceiling =%d", __func__, value);
}

static void set_app_profile_max_cpu_freq_percent(struct powerhal_info *pInfo, int percent)
{
    int targetMaxFreq = pInfo->cpu0_max_frequency;

    if (percent == PM_QOS_DEFAULT_VALUE)
        percent = 100;

    if (percent > 0 && percent <= 100) {
        if (pInfo->cpu0_max_frequency >= 0) {
            targetMaxFreq = percent * pInfo->cpu0_max_frequency / 100;
        }
    } else {
        ALOGW("%s: invalid percentage =%d", __func__, percent);
    }

    set_app_profile_max_cpu_freq(pInfo, targetMaxFreq);
}

static void set_app_profile_max_online_cpus(struct powerhal_info *pInfo, int value)
{
    if (value <= 0)
        value = pInfo->defaults.core_cap;

    if (pInfo->fds.app_max_online_cpus >= 0) {
        close(pInfo->fds.app_max_online_cpus);
        pInfo->fds.app_max_online_cpus = -1;
    }
    pInfo->fds.app_max_online_cpus =
        pInfo->mTimeoutPoker->requestPmQos(PMQOS_CONSTRAINT_ONLINE_CPUS, PM_QOS_APP_PROFILE_PRIORITY, value, PM_QOS_DEFAULT_VALUE);

    ALOGV("%s: set max online CPU core =%d", __func__, value);
}

static void set_app_profile_min_online_cpus(struct powerhal_info *pInfo, int value)
{
    if (pInfo->fds.app_min_online_cpus >= 0) {
        close(pInfo->fds.app_min_online_cpus);
        pInfo->fds.app_min_online_cpus = -1;
    }
    pInfo->fds.app_min_online_cpus =
        pInfo->mTimeoutPoker->requestPmQos(PMQOS_CONSTRAINT_ONLINE_CPUS, PM_QOS_APP_PROFILE_PRIORITY, PM_QOS_DEFAULT_VALUE, value);

    ALOGV("%s: set min online CPU core =%d", __func__, value);
}

static void set_app_profile_min_gpu_freq(struct powerhal_info *pInfo, int value)
{
    if (pInfo->fds.app_min_gpu >= 0) {
        close(pInfo->fds.app_min_gpu);
        pInfo->fds.app_min_gpu = -1;
    }
    if (value)
        value = 0;
    else
        value = INT_MAX;

    pInfo->fds.app_min_gpu =
        pInfo->mTimeoutPoker->requestPmQos(PMQOS_CONSTRAINT_GPU_FREQ, PM_QOS_APP_PROFILE_PRIORITY, PM_QOS_DEFAULT_VALUE, value);
}

static void set_prism_control_enable(struct powerhal_info *pInfo, int value)
{
    if (value)
        set_property_int(PRISM_CONTROL_PROP, 1);
    else
        set_property_int(PRISM_CONTROL_PROP, 0);
    ALOGV("%s: set prism enable =%d", __func__, value);
}

static void set_app_profile_max_gpu_freq(struct powerhal_info *pInfo, int value)
{
    if (value <= 0)
        value = pInfo->defaults.gpu_cap;

#ifndef GPU_IS_LEGACY
    if (pInfo->fds.app_max_gpu >= 0) {
        close(pInfo->fds.app_max_gpu);
        pInfo->fds.app_max_gpu = -1;
    }
    pInfo->fds.app_max_gpu =
        pInfo->mTimeoutPoker->requestPmQos(PMQOS_CONSTRAINT_GPU_FREQ, PM_QOS_APP_PROFILE_PRIORITY, value, PM_QOS_DEFAULT_VALUE);
#else
    /* legacy sysfs nodes to throttle GPU on "pre-T124" chips */
    sysfs_write_int("sys/kernel/tegra_cap/cbus_cap_state", 1);
    sysfs_write_int("sys/kernel/tegra_cap/cbus_cap_level", value);
#endif
}

static void set_pbc_power(struct powerhal_info *pInfo, int value)
{
    if (value < 0)
        value = pInfo->defaults.power_cap;

    set_property_int(POWER_CAP_PROP, value);
}

static void set_fan_cap(struct powerhal_info *pInfo, int value)
{
    if (!pInfo->features.fan)
        return;

    if (value < 0)
        value = pInfo->defaults.fan_cap;

    sysfs_write_int("/sys/devices/platform/pwm-fan/pwm_cap", value);
}

static void set_camera_cpu_freq_min(struct powerhal_info *pInfo, int value)
{
    ALOGV("%s: %d", __func__, value);
    if (value > 0)
    {
        if (pInfo->camera_power.fd_cpu_freq_min != -1)
        {
            close(pInfo->camera_power.fd_cpu_freq_min);
        }
        pInfo->camera_power.fd_cpu_freq_min =
            pInfo->mTimeoutPoker->requestPmQos("/dev/cpu_freq_min", value);
    }
}

static void set_camera_cpu_freq_max(struct powerhal_info *pInfo, int value)
{
    ALOGV("%s: %d", __func__, value);
    if (value > 0)
    {
        if (pInfo->camera_power.fd_cpu_freq_max != -1)
        {
            close(pInfo->camera_power.fd_cpu_freq_max);
        }
        pInfo->camera_power.fd_cpu_freq_max =
            pInfo->mTimeoutPoker->requestPmQos("/dev/cpu_freq_max", value);
    }
}

static void set_camera_min_online_cpus(struct powerhal_info *pInfo, int value)
{
    ALOGV("%s: %d", __func__, value);
    if (pInfo->camera_power.fd_min_online_cpus != -1)
    {
        close(pInfo->camera_power.fd_min_online_cpus);
    }
    pInfo->camera_power.fd_min_online_cpus =
        pInfo->mTimeoutPoker->requestPmQos("/dev/min_online_cpus", value);
}

static void set_camera_max_online_cpus(struct powerhal_info *pInfo, int value)
{
    ALOGV("%s: %d", __func__, value);
    if (value > 0)
    {
        if (pInfo->camera_power.fd_max_online_cpus != -1)
        {
            close(pInfo->camera_power.fd_max_online_cpus);
        }
        pInfo->camera_power.fd_max_online_cpus =
            pInfo->mTimeoutPoker->requestPmQos("/dev/max_online_cpus", value);
    }
}

static void set_camera_fps(struct powerhal_info *pInfo)
{
    int sts;

    if (pInfo->camera_power.fd_gpu == -1)
    {
        pInfo->camera_power.fd_gpu = NvOsSetFpsTarget(pInfo->camera_power.target_fps);

        if (pInfo->camera_power.fd_gpu == -1)
        {
            ALOGE("fail to set camera perf target");
            return;
        }
    }
    else
    {
        sts = NvOsModifyFpsTarget(pInfo->camera_power.fd_gpu, pInfo->camera_power.target_fps);

        if (sts == -1)
        {
            ALOGE("fail to modify camera perf target");
            return;
        }
    }
    ALOGV("%s: set %d fps to GPU FPS target fd=%d", __func__,
        pInfo->camera_power.target_fps, pInfo->camera_power.fd_gpu);
}

static void set_camera_single_phs_hint(struct powerhal_info *pInfo, NvHintType hintType, NvU32 value)
{
    int sts;

    if (!pInfo->NvVaSendThroughputHints)
        return;

    sts = pInfo->NvVaSendThroughputHints(NvUsecase_camera,
                                         hintType,
                                         value,
                                         SLEEP_INTERVAL_SECS*1000,
                                         NULL);

    if (sts)
    {
        ALOGE("%s: fail to set hint %d, err=%d",  __func__, hintType, sts);
    }
    else
    {
        ALOGV("%s: set hint %d to value=%d", __func__, hintType, value);
    }
}

static void set_camera_phs_hints(struct powerhal_info *pInfo, camera_cap_t *cap)
{
    if (cap->minCpuHint)
    {
        set_camera_single_phs_hint(pInfo, NvHintType_MinCPU, cap->minCpuHint);
    }

    if (cap->maxCpuHint)
    {
        set_camera_single_phs_hint(pInfo, NvHintType_MaxCPU, cap->maxCpuHint);
    }

    if (cap->minGpuHint)
    {
        set_camera_single_phs_hint(pInfo, NvHintType_MinGPU, cap->minGpuHint);
    }

    if (cap->maxGpuHint)
    {
        set_camera_single_phs_hint(pInfo, NvHintType_MaxGPU, cap->maxGpuHint);
    }

    if (cap->fpsHint)
    {
#ifdef PHS_DEBUG
        pInfo->camera_power.target_fps = cap->fpsHint;
#else
        set_camera_single_phs_hint(NvHintType_FramerateTarget, cap->fpsHint);
#endif
    }
}

static void reset_camera_hint(struct powerhal_info *pInfo)
{
    if (pInfo->camera_power.fd_gpu != -1)
    {
        ALOGV("%s: cancel camera perf target", __func__);
        NvOsCancelFpsTarget(pInfo->camera_power.fd_gpu);
        pInfo->camera_power.fd_gpu = -1;
    }
    pInfo->camera_power.target_fps = 0;

    if (pInfo->camera_power.fd_cpu_freq_min != -1)
    {
        close(pInfo->camera_power.fd_cpu_freq_min);
        pInfo->camera_power.fd_cpu_freq_min = -1;
    }

    if (pInfo->camera_power.fd_cpu_freq_max != -1)
    {
        close(pInfo->camera_power.fd_cpu_freq_max);
        pInfo->camera_power.fd_cpu_freq_max = -1;
    }

    if (pInfo->fds.app_min_freq >= 0) {
        close(pInfo->fds.app_min_freq);
        pInfo->fds.app_min_freq = -1;
    }

    if (pInfo->camera_power.fd_min_online_cpus != -1)
    {
        close(pInfo->camera_power.fd_min_online_cpus);
        pInfo->camera_power.fd_min_online_cpus = -1;
    }

    if (pInfo->camera_power.fd_max_online_cpus != -1)
    {
        close(pInfo->camera_power.fd_max_online_cpus);
        pInfo->camera_power.fd_max_online_cpus = -1;
    }

    if (pInfo->camera_power.usecase_index != -1)
    {
        pInfo->camera_power.usecase_index = -1;
        if (pInfo->NvCancelThroughputHints)
            pInfo->NvCancelThroughputHints(NvUsecase_camera, NVHINT_DEFAULT_TAG);
    }
}

static void *regular_hints_threadfunc(void *powerInfo)
{
    int err = 0;
    struct powerhal_info *pInfo;
    pInfo = (struct powerhal_info *)powerInfo;
    struct timespec timeout;
    struct timeval tv;

    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &timeout);
        timeout.tv_sec += SLEEP_INTERVAL_SECS;
        pthread_mutex_lock(&pInfo->wait_mutex);
        if (pInfo->exit_hints_thread == true)
        {
            pthread_mutex_unlock(&pInfo->wait_mutex);
            break;
        }

#ifdef HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC
        err = pthread_cond_timedwait_monotonic(&pInfo->wait_cond, &pInfo->wait_mutex, &timeout);
#else
        err = pthread_cond_timedwait(&pInfo->wait_cond, &pInfo->wait_mutex, &timeout);
#endif

        pthread_mutex_unlock(&pInfo->wait_mutex);
        // loop back if timedout
        if (err == ETIMEDOUT)
        {
            if (pInfo->camera_power.target_fps)
            {
                ALOGV("Woken up by Timeout, set fps");
                set_camera_fps(pInfo);
            }

            if ((pInfo->camera_power.usecase_index >= CAMERA_STILL_PREVIEW) &&
                (pInfo->camera_power.usecase_index < CAMERA_USECASE_COUNT))
            {
                ALOGV("Woken up by Timeout, set phs hints");
                set_camera_phs_hints(pInfo, &(pInfo->camera_power.cam_cap[pInfo->camera_power.usecase_index]));
            }

            continue;
        }
        else
        {
            ALOGV("Woken up by signal, exit");
            break;
        }
    }
    ALOGV("Exiting regular hints thread");
    return NULL;

}

static void wait_for_regular_hints_thread(struct powerhal_info *pInfo)
{
    int err = 0;
    ALOGV("Signal regular hints thread");
    if (pInfo->regular_hints_thread)
    {
        pthread_mutex_lock(&pInfo->wait_mutex);
        pInfo->exit_hints_thread = true;
        pthread_mutex_unlock(&pInfo->wait_mutex);
        err = pthread_cond_signal(&pInfo->wait_cond);
    }
    else
    {
        ALOGV("Thread already exited");
        return;
    }
    if (err)
    {
        ALOGE("%s: condition variable not initialized", __func__);
    }
    pthread_join(pInfo->regular_hints_thread, NULL);
    pInfo->regular_hints_thread = 0;
    pInfo->exit_hints_thread = false;
}


static void send_regular_hints(struct powerhal_info *pInfo)
{
    int err = 0;

    // don't create another thread if hints are already being sent
    pthread_mutex_lock(&pInfo->wait_mutex);
    if (pInfo->regular_hints_thread)
    {
        pthread_mutex_unlock(&pInfo->wait_mutex);
        return;
    }
    ALOGV("Creating regular hints thread");
    err = pthread_create(&pInfo->regular_hints_thread, NULL,
            regular_hints_threadfunc, (void *)pInfo);

    pthread_mutex_unlock(&pInfo->wait_mutex);
    if (err)
    {
        ALOGE("%s: failed to create thread, errno = %d", __func__, errno);
    }
}

static void set_camera_hint(struct powerhal_info *pInfo, camera_hint_t *data)
{
    camera_cap_t *cap = NULL;

    ALOGV("%s: setting camera_hint hint = %d", __func__, data[0]);

    switch (data[0]) {
        /* POWER and PERF will be implemented later */
        case CAMERA_HINT_STILL_PREVIEW_POWER:
            pInfo->camera_power.usecase_index = CAMERA_STILL_PREVIEW;
            cap = &(pInfo->camera_power.cam_cap[CAMERA_STILL_PREVIEW]);
            break;
        case CAMERA_HINT_VIDEO_PREVIEW_POWER:
            pInfo->camera_power.usecase_index = CAMERA_VIDEO_PREVIEW;
            cap = &(pInfo->camera_power.cam_cap[CAMERA_VIDEO_PREVIEW]);
            break;
        case CAMERA_HINT_VIDEO_RECORD_POWER:
            pInfo->camera_power.usecase_index = CAMERA_VIDEO_RECORD;
            cap = &(pInfo->camera_power.cam_cap[CAMERA_VIDEO_RECORD]);
            break;
        case CAMERA_HINT_HIGH_FPS_VIDEO_RECORD_POWER:
            pInfo->camera_power.usecase_index = CAMERA_VIDEO_RECORD_HIGH_FPS;
            cap = &(pInfo->camera_power.cam_cap[CAMERA_VIDEO_RECORD_HIGH_FPS]);
            break;
        case CAMERA_HINT_PERF:
            // boost CPU freq to highest for 1s
            pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_CPU_FREQ,
                                                    PM_QOS_BOOST_PRIORITY,
                                                    PM_QOS_DEFAULT_VALUE,
                                                    pInfo->available_frequencies[pInfo->num_available_frequencies - 1],
                                                    s2ns(1));
            pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_ONLINE_CPUS,
                                                    PM_QOS_BOOST_PRIORITY,
                                                    PM_QOS_DEFAULT_VALUE,
                                                    2,
                                                    s2ns(1));
            break;
        case CAMERA_HINT_FPS:
            pInfo->camera_power.target_fps = CAMERA_TARGET_FPS;
            set_camera_fps(pInfo);
            ALOGV("%s: set_camera: target_fps = %d", __func__, pInfo->camera_power.target_fps);
            break;
        case CAMERA_HINT_RESET:
            wait_for_regular_hints_thread(pInfo);
            reset_camera_hint(pInfo);
        default:
            break;
    }

    if (cap)
    {
        if (cap->minFreq)
        {
            // floor CPU freq
            set_camera_cpu_freq_min(pInfo, cap->minFreq);
        }

        if (cap->freq)
        {
            // cap CPU freq
            set_camera_cpu_freq_max(pInfo, cap->freq);
        }

        if (cap->min_online_cpus)
        {
            // cap the number of CPU
            set_camera_min_online_cpus(pInfo, cap->min_online_cpus);
        }

        if (cap->max_online_cpus)
        {
            // cap the number of CPU
            set_camera_max_online_cpus(pInfo, cap->max_online_cpus);
        }

        set_camera_phs_hints(pInfo, cap);
    }

    // launch regular hints thread
    if (pInfo->camera_power.target_fps)
    {
        //lauch thread for fps
        ALOGV("%s: set_camera: lauch regular thread for fps", __func__);
        send_regular_hints(pInfo);
    }
    else
    {
        //lauch thread for phs hints
        if(cap)
        {
            if (cap->minCpuHint || cap->maxCpuHint || cap->minGpuHint ||
                cap->maxGpuHint || cap->fpsHint)
            {
                ALOGV("%s: set_camera: lauch regular thread for phs hints", __func__);
                send_regular_hints(pInfo);
            }
        }
    }
}

static void app_profile_set(struct powerhal_info *pInfo, app_profile_knob *data)
{
    int i;

    for (i = 0; i < APP_PROFILE_COUNT; i++)
    {
        switch (i) {
            case APP_PROFILE_CPU_SCALING_MIN_FREQ:
                set_app_profile_min_cpu_freq(pInfo, data[i]);
                break;
            case APP_PROFILE_CPU_MAX_NORMAL_FREQ_IN_PERCENTAGE:
                //As user operation take the highest priority
                //Other cpu max freq related control should be before it.
                set_app_profile_max_cpu_freq_percent(pInfo, data[i]);
                break;
            case APP_PROFILE_CPU_MAX_CORE:
                set_app_profile_max_online_cpus(pInfo, data[i]);
                break;
            case APP_PROFILE_GPU_CBUS_CAP_LEVEL:
                set_app_profile_max_gpu_freq(pInfo, data[i]);
                break;
            case APP_PROFILE_GPU_SCALING:
                set_app_profile_min_gpu_freq(pInfo, data[i]);
                break;
            case APP_PROFILE_PRISM_CONTROL_ENABLE:
                set_prism_control_enable(pInfo, data[i]);
                break;
            case APP_PROFILE_CPU_MIN_CORE:
                set_app_profile_min_online_cpus(pInfo, data[i]);
                break;
            default:
                break;
        }
    }
}

void common_power_init(struct power_module *module, struct powerhal_info *pInfo)
{
    common_power_open(pInfo);

    pInfo->ftrace_enable = get_property_bool("nvidia.hwc.ftrace_enable", false);

    // Boost to max frequency on initialization to decrease boot time
    pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_CPU_FREQ,
                                            PM_QOS_BOOST_PRIORITY,
                                            PM_QOS_DEFAULT_VALUE,
                                            pInfo->available_frequencies[pInfo->num_available_frequencies - 1],
                                            s2ns(15));

    pInfo->switch_cpu_emc_limit_enabled = sysfs_exists(CPU_EMC_RATIO_SRC_NODE);
}

void common_power_set_interactive(struct power_module *module, struct powerhal_info *pInfo, int on)
{
    int i;
    int dev_id;
    char path[80];
    const char* state = (0 == on)?"0":"1";
    int power_mode = -1;

    sysfs_write("/sys/devices/platform/host1x/nvavp/boost_sclk", state);

    if (0 != pInfo) {
        for (i = 0; i < pInfo->input_cnt; i++) {
            if (0 == pInfo->input_devs)
                dev_id = i;
            else if (-1 == pInfo->input_devs[i].dev_id)
                continue;
            else
                dev_id = pInfo->input_devs[i].dev_id;
            snprintf(path, sizeof(path), "/sys/class/input/input%d/enabled", dev_id);
            if (!access(path, W_OK)) {
                if (0 == on)
                    ALOGI("Disabling input device:%d", dev_id);
                else
                    ALOGI("Enabling input device:%d", dev_id);
                sysfs_write(path, state);
            }
        }

        if(pInfo->switch_cpu_emc_limit_enabled) {
            sysfs_write_int(CPU_EMC_RATIO_SRC_NODE, on);
        }
    }

#ifdef POWER_MODE_SET_INTERACTIVE
    if (on) {
        power_mode = get_system_power_mode();
        if (power_mode < NVCPL_HINT_MAX_PERF || power_mode > NVCPL_HINT_COUNT) {
            ALOGV("%s: no system power mode info, take optimized settings", __func__);
            power_mode = NVCPL_HINT_OPT_PERF;
        }
    } else {
        power_mode = NVCPL_HINT_COUNT;
    }
    set_interactive_governor(power_mode);
#else
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/hispeed_freq", (on == 0)?"420000":"624000");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/target_loads", (on == 0)?"45 312000:75 564000:85":"65 228000:75 624000:85");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/above_hispeed_delay", (on == 0)?"80000":"19000");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/timer_rate", (on == 0)?"300000":"20000");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/boost_factor", (on == 0)?"2":"0");
#endif
}

#ifdef POWER_MODE_SET_INTERACTIVE
static int get_system_power_mode(void)
{
    char value[PROPERTY_VALUE_MAX] = { 0 };
    int power_mode = -1;

    property_get("persist.sys.NV_POWER_MODE", value, "");
    if (value[0] != '\0')
    {
        power_mode = atoi(value);
    }

    if (get_property_bool("persist.sys.NV_ECO.STATE.ISECO", false))
    {
        power_mode = NVCPL_HINT_BAT_SAVE;
    }

    return power_mode;
}

static void __sysfs_write(const char *file, const char *data)
{
    if (data != NULL)
    {
        sysfs_write(file, data);
    }
}

static void set_interactive_governor(int mode)
{
    __sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/hispeed_freq",
            interactive_data_array[mode].hispeed_freq);
    __sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/target_loads",
            interactive_data_array[mode].target_loads);
    __sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/above_hispeed_delay",
            interactive_data_array[mode].above_hispeed_delay);
    __sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/timer_rate",
            interactive_data_array[mode].timer_rate);
    __sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/boost_factor",
            interactive_data_array[mode].boost_factor);
    __sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/min_sample_time",
            interactive_data_array[mode].min_sample_time);
    __sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/go_hispeed_load",
            interactive_data_array[mode].go_hispeed_load);
}

static void set_power_mode_hint(struct powerhal_info *pInfo, nvcpl_hint_t *data)
{
    int mode = data[0];
    int status;
    char value[4] = { 0 };

    if (mode < NVCPL_HINT_MAX_PERF || mode > NVCPL_HINT_COUNT)
    {
        ALOGE("%s: invalid hint mode = %d", __func__, mode);
        return;
    }

    // only set interactive governor parameters when display on
    sysfs_read("/sys/class/backlight/pwm-backlight/brightness", value, sizeof(value));
    status = atoi(value);

    if (status)
    {
        set_interactive_governor(mode);
    }

}
#endif

void common_power_hint(struct power_module *module, struct powerhal_info *pInfo,
                            power_hint_t hint, void *data)
{
    uint64_t t;

    if (!pInfo)
        return;

    if (check_hint(pInfo, hint, &t) < 0)
        return;

    switch (hint) {
    case POWER_HINT_VSYNC:
        if (data)
            set_vsync_min_cpu_freq(pInfo, *(int *)data);
        break;
    case POWER_HINT_INTERACTION:
        break;
    case POWER_HINT_MULTITHREAD_BOOST:
        // Boost to 4 cores
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_ONLINE_CPUS,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                4,
                                                s2ns(2));
        break;
    case POWER_HINT_APP_LAUNCH:
        // Boost to 1.2Ghz dual core
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_CPU_FREQ,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                INT_MAX,
                                                ms2ns(1500));
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_ONLINE_CPUS,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                2,
                                                ms2ns(1500));
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_GPU_FREQ,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                180000,
                                                ms2ns(1500));
        pInfo->mTimeoutPoker->requestPmQosTimed("/dev/emc_freq_min",
                                                792000,
                                                ms2ns(1500));
        break;
    case POWER_HINT_APP_PROFILE:
        if (data) {
            app_profile_set(pInfo, (app_profile_knob*)data);
        }
        break;
    case POWER_HINT_SHIELD_STREAMING:
        // set minimum 2 CPU core
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_ONLINE_CPUS,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                2,
                                                s2ns(1));
        // Set minimum CPU freq to 816 MHz
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_CPU_FREQ,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                816000,
                                                s2ns(1));
        break;
    case POWER_HINT_HIGH_RES_VIDEO:
        // set minimum 2 CPU core
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_ONLINE_CPUS,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                2,
                                                s2ns(1));
        // Set minimum CPU freq to 816 MHz
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_CPU_FREQ,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                816000,
                                                s2ns(1));
        break;
    case POWER_HINT_VIDEO_DECODE:
        // set minimum 1 CPU core
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_ONLINE_CPUS,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                1,
                                                s2ns(1));
        // Set minimum CPU freq to 710 MHz
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_CPU_FREQ,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                710000,
                                                s2ns(1));
        break;
    case POWER_HINT_MIRACAST:
        // Boost to 816 Mhz frequency for one second
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_CPU_FREQ,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                816000,
                                                s2ns(1));
    case POWER_HINT_CAMERA:
        set_camera_hint(pInfo, (camera_hint_t*)data);

        break;
    case POWER_HINT_DISPLAY_ROTATION:
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_CPU_FREQ,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                1500000,
                                                s2ns(2));
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_ONLINE_CPUS,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                2,
                                                s2ns(2));
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_GPU_FREQ,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                252000,
                                                s2ns(2));
        pInfo->mTimeoutPoker->requestPmQosTimed("/dev/emc_freq_min",
                                                 400000,
                                                 s2ns(2));
        break;
    case POWER_HINT_AUDIO_SPEAKER:
        // Boost to 512 Mhz frequency for one second
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_CPU_FREQ,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                512000,
                                                s2ns(1));
        break;
    case POWER_HINT_AUDIO_OTHER:
        // Boost to 512 Mhz frequency for one second
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_CPU_FREQ,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                512000,
                                                s2ns(1));
        break;
    case POWER_HINT_AUDIO_LOW_LATENCY:
        // Boost to 1 Ghz frequency for one second
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_CPU_FREQ,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                1000000,
                                                s2ns(1));
        pInfo->mTimeoutPoker->requestPmQosTimed(PMQOS_CONSTRAINT_ONLINE_CPUS,
                                                PM_QOS_BOOST_PRIORITY,
                                                PM_QOS_DEFAULT_VALUE,
                                                4,
                                                s2ns(1));
        pInfo->mTimeoutPoker->requestPmQosTimed("/dev/emc_freq_min",
                                                 300000,
                                                 s2ns(1));
        break;
    case POWER_HINT_POWER_MODE:
#ifdef POWER_MODE_SET_INTERACTIVE
        // Set interactive governor parameters according to power mode
        set_power_mode_hint(pInfo, (nvcpl_hint_t *)data);
#endif
        break;
    default:
        ALOGE("Unknown power hint: 0x%x", hint);
        break;
    }

    pInfo->hint_time[hint] = t;
}

/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (c) 2012-2015, NVIDIA CORPORATION.  All rights reserved.
 *
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

#ifndef COMMON_POWER_HAL_H
#define COMMON_POWER_HAL_H

#include <hardware/hardware.h>
#include <hardware/power.h>

#include "powerhal_utils.h"
#include "timeoutpoker.h"
#include <semaphore.h>

#define MAX_CHARS 32
#define MAX_INPUT_DEV_COUNT 12
#define MAX_USE_CASE_STRING_SIZE 80
#define MAX_POWER_HINT_COUNT 0x1f

#define CAMERA_TARGET_FPS 30
#define DEFAULT_MIN_ONLINE_CPUS     2
#define DEFAULT_MAX_ONLINE_CPUS     0
#define DEFAULT_FREQ                700

#define CAMERA_TARGET_FPS 30
#define POWER_CAP_PROP "persist.sys.NV_PBC_PWR_LIMIT"
#define SLEEP_INTERVAL_SECS 1

//sys node control entry
#define SYS_NODE_PRISM_ENABLE           "/sys/devices/platform/host1x/tegradc.0/smartdimmer/enable"
#define SYS_NODE_CPU0_MAX_FREQ          "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"

//PMQOS control entry
#define PMQOS_MAX_ONLINE_CPUS           "/dev/max_online_cpus"
#define PMQOS_MAX_CPU_FREQ              "/dev/cpu_freq_max"
#define PMQOS_CONSTRAINT_CPU_FREQ       "/dev/constraint_cpu_freq"
#define PMQOS_CONSTRAINT_GPU_FREQ       "/dev/constraint_gpu_freq"
#define PMQOS_CONSTRAINT_ONLINE_CPUS    "/dev/constraint_online_cpus"

//Default value to align with kernel pm qos
#define PM_QOS_DEFAULT_VALUE		-1

#define PRISM_CONTROL_PROP              "persist.sys.didim.enable"

#define PM_QOS_BOOST_PRIORITY 35
#define PM_QOS_APP_PROFILE_PRIORITY  40

struct input_dev_map {
    int dev_id;
    const char* dev_name;
};

typedef enum camera_usecase {
    CAMERA_STILL_PREVIEW = 0,
    CAMERA_VIDEO_PREVIEW,
    CAMERA_VIDEO_RECORD,
    CAMERA_VIDEO_RECORD_HIGH_FPS,
    CAMERA_USECASE_COUNT
} camera_usecase_t;

typedef struct camera_cap {
    int min_online_cpus;
    int max_online_cpus;
    int freq;
    int minFreq;
    int minCpuHint;
    int maxCpuHint;
    int minGpuHint;
    int maxGpuHint;
    int fpsHint;
} camera_cap_t;

typedef struct interactive_data {
    const char *hispeed_freq;
    const char *target_loads;
    const char *above_hispeed_delay;
    const char *timer_rate;
    const char *boost_factor;
    const char *min_sample_time;
    const char *go_hispeed_load;
} interactive_data_t;

typedef int (*sendhints_fn_t)(uint32_t client_tag, ...);
typedef void (*cancelhints_fn_t)(uint32_t usecase, uint32_t client_tag);

struct powerhal_info {
    TimeoutPoker* mTimeoutPoker;

    int *available_frequencies;
    int num_available_frequencies;

    /* Maximum LP CPU frequency */
    int lp_max_frequency;

    int interaction_boost_frequency;
    int animation_boost_frequency;

    /* maximum frequency for the current cpufreq policy */
    int cpu0_max_frequency;

    bool ftrace_enable;

    /* Number of devices requesting Power HAL service */
    int input_cnt;

    /* Holds input devices */
    struct input_dev_map* input_devs;

    /* Time last hint was sent - in usec */
    uint64_t hint_time[MAX_POWER_HINT_COUNT];
    uint64_t hint_interval[MAX_POWER_HINT_COUNT];

    /* waiting condvar regular hints thread */
    pthread_cond_t wait_cond;

    /* waiting mutex regular hints thread */
    pthread_mutex_t wait_mutex;

    /* regular hints thread handle */
    pthread_t regular_hints_thread;

    /* regular hints thread handle */
    bool exit_hints_thread;

    /* AppProfile defaults */
    struct {
        int min_freq;
        int max_freq;
        int core_cap;
        int gpu_cap;
        int fan_cap;
        int power_cap;
    } defaults;

    /* Features on platform */
    struct {
        bool fan;
    } features;

    /* File descriptors used for hints and app profiles */
    struct {
        int app_min_freq;
        int app_max_freq;
        int app_max_online_cpus;
        int app_min_online_cpus;
        int app_max_gpu;
        int app_min_gpu;
        int vsync_min_cpu;
    } fds;

    /* Camera power hint struct*/
    struct {
        /* handle for dynamic loaded library */
        void *handle;
        /* File descriptor used to set GPU target FPS */
        int fd_gpu;
        /* File descriptor used to set CPU min frequency */
        int fd_cpu_freq_min;
        /* File descriptor used to set CPU max frequency */
        int fd_cpu_freq_max;
        /* File descriptor used to set min CPUs online */
        int fd_min_online_cpus;
        /* File descriptor used to set max CPUs online */
        int fd_max_online_cpus;

        int target_fps;

        camera_cap_t *cam_cap;
        int usecase_index;
    } camera_power;

    /* PHS hint function pointers. Loaded in runtime since powerhal can't
     * depend on libphs in link time. */
    void *libphs_handle;
    sendhints_fn_t NvVaSendThroughputHints;
    cancelhints_fn_t NvCancelThroughputHints;

    /* Switching CPU/EMC freq ratio based on display state */
    bool switch_cpu_emc_limit_enabled;
};

/* Opens power hw module */
void common_power_open(struct powerhal_info *pInfo);

/* Power management setup action at startup.
 * Such as to set default cpufreq parameters.
 */
void common_power_init(struct power_module *module, struct powerhal_info *pInfo);

/* Power management action,
 * upon the system entering interactive state and ready for interaction,
 * often with UI devices
 * OR
 * non-interactive state the system appears asleep, displayi/touch usually turned off.
*/
void common_power_set_interactive(struct power_module *module,
                                    struct powerhal_info *pInfo, int on);

/* PowerHint called to pass hints on power requirements, which
 * may result in adjustment of power/performance parameters of the
 * cpufreq governor and other controls.
*/
void common_power_hint(struct power_module *module, struct powerhal_info *pInfo,
                            power_hint_t hint, void *data);

/*
 * Initialize struct camera_power. Copy platform dependent CPU config to
 * cam_cap in struct camera_power.
 */
void common_power_camera_init(struct powerhal_info *pInfo, camera_cap_t *cap);

#endif  //COMMON_POWER_HAL_H


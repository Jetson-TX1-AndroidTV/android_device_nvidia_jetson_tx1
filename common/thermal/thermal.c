/*
 * Copyright (C) 2016 The Android Open Source Project
 * Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
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

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


#define LOG_TAG "ThermalHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/thermal.h>

#include "thermal.h"

#define CPU_LABEL               "CPU"
#define CPU_USAGE_FILE          "/proc/stat"
#define THERMAL_ROOT_DIR        "/sys/class/thermal"
#define TEMPERATURE_FILE_FORMAT "/sys/class/thermal/thermal_zone%d/temp"
#define THERMAL_DIR             "thermal_zone"
#define CPU_ONLINE_FILE_FORMAT  "/sys/devices/system/cpu/cpu%d/online"

#define RPM_FAN_FILE            "/sys/kernel/debug/tegra_fan/cur_rpm"

extern thermal_desc_t platform_data[];
extern int platform_data_count;
extern int num_cpus_total;

static int get_temperature_paths()
{
    DIR *dir;
    struct dirent *de;
    char file_name[MAX_LENGTH];
    char current_label[MAX_LENGTH];
    FILE *file;
    dir = opendir(THERMAL_ROOT_DIR);

    while ((de = readdir(dir)) != NULL) {
        if (!strncmp(de->d_name, THERMAL_DIR, strlen(THERMAL_DIR))) {
            snprintf(file_name, MAX_LENGTH, "%s/%s/type", THERMAL_ROOT_DIR, de->d_name);
        }
        file = fopen(file_name, "r");

        if (file == NULL)
            continue;

        if (1 != fscanf(file, "%s", (char *)&current_label)) {
            fclose(file);
            continue;
        }
        fclose(file);

        /* If type matches sensor label in platform data, cache it in temperature_path */
        for (int i = 0; i < platform_data_count; i++) {
            if (platform_data[i].temperature_path)
                continue;

            if (!strcmp(platform_data[i].sensor_label, current_label)) {
                snprintf(file_name, MAX_LENGTH, "%s/%s/temp", THERMAL_ROOT_DIR, de->d_name);
                platform_data[i].temperature_path = malloc(strlen(file_name) + 1);

                if (!platform_data[i].temperature_path)
                    return -ENOMEM;

                strcpy(platform_data[i].temperature_path, file_name);
            }
        }
    }

    closedir(dir);

    return 0;
}

int read_temperature(const thermal_desc_t *in, temperature_t *out, int size)
{
    char temperature_path[MAX_LENGTH];
    FILE *fp;
    float temp;
    int ret;

    if (size < 1)
        return -ENOMEM;

    if (in->temperature_path)
        fp = fopen(in->temperature_path, "r");
    else
        return -ENOENT;

    if (fp == NULL)
        return -ENOENT;

    ret = fscanf(fp, "%f", &temp);
    fclose(fp);

    if (ret != 1)
        return -ENOENT;

    (*out) = (temperature_t) {
        .type = in->type,
        .name = in->name,
        .current_value = temp * in->multiplier,
        .throttling_threshold = in->throttling_threshold,
        .shutdown_threshold = in->shutdown_threshold,
        .vr_throttling_threshold = in->vr_throttling_threshold
    };

    // Added "1" temperature to list
    return 1;
}

int read_cluster_temperature(const thermal_desc_t *in, temperature_t *out, int size)
{
    int ret;

    if (size < in->cores)
        return -ENOMEM;

    if ((ret = read_temperature(in, out, size)) < 0)
        return ret;

    out->name = in->core_names[0];

    for (int i = 1; i < in->cores; i++) {
        temperature_t *next = &out[i];
        *next = *out;
        next->name = in->core_names[i];
    }

    return in->cores;
}

static ssize_t get_temperatures(thermal_module_t *module, temperature_t *list, size_t size) {
    size_t idx = 0;
    int ret = 0;

    if (list == NULL) {
        for (int i = 0; i < platform_data_count; i++)
            ret += platform_data[i].cores ? platform_data[i].cores : 1;
        return ret;
    }

    if (size == 0)
        return -ENOMEM;

    ret = get_temperature_paths();

    if (ret < 0)
        goto out;

    for (int i = 0; i < platform_data_count; i++) {
        if (platform_data[i].read_temperature)
            ret = platform_data[i].read_temperature(&platform_data[i], &list[idx], size - idx);
        else
            ret = read_temperature(&platform_data[i], &list[idx], size - idx);

        if (ret < 0)
            goto out;
        else
            idx += ret;
    }

    /* Success.  Return number of entries */
    ret = idx;

out:
    for (int i = 0; i < platform_data_count; i++) {
        if (platform_data[i].temperature_path) {
            free(platform_data[i].temperature_path);
            platform_data[i].temperature_path = NULL;
        }
    }

    return ret;
}

static ssize_t get_cpu_usages(thermal_module_t *module, cpu_usage_t *list) {
    int vals, cpu_num, online;
    ssize_t read;
    uint64_t user, nice, system, idle, active, total;
    char *line = NULL;
    size_t len = 0;
    size_t size = 0;
    char file_name[MAX_LENGTH];
    FILE *cpu_file;
    FILE *file;

    if (list == NULL) {
        return num_cpus_total;
    }

    file = fopen(CPU_USAGE_FILE, "r");
    if (file == NULL) {
        ALOGE("%s: failed to open: %s", __func__, strerror(errno));
        return -errno;
    }

    while ((read = getline(&line, &len, file)) != -1) {
        // Skip non "cpu[0-9]" lines.
        if (strnlen(line, read) < 4 || strncmp(line, "cpu", 3) != 0 || !isdigit(line[3])) {
            free(line);
            line = NULL;
            len = 0;
            continue;
        }
        vals = sscanf(line, "cpu%d %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64, &cpu_num, &user,
                &nice, &system, &idle);

        free(line);
        line = NULL;
        len = 0;

        if (vals != 5) {
            ALOGE("%s: failed to read CPU information from file: %s", __func__, strerror(errno));
            fclose(file);
            return errno ? -errno : -EIO;
        }

        active = user + nice + system;
        total = active + idle;

        // Read online CPU information.
        snprintf(file_name, MAX_LENGTH, CPU_ONLINE_FILE_FORMAT, cpu_num);
        cpu_file = fopen(file_name, "r");
        online = 0;
        if (cpu_file == NULL) {
            ALOGE("%s: failed to open file: %s (%s)", __func__, file_name, strerror(errno));
            // /sys/devices/system/cpu/cpu0/online is missing on some systems, because cpu0 can't
            // be offline.
            online = cpu_num == 0;
        } else if (1 != fscanf(cpu_file, "%d", &online)) {
            ALOGE("%s: failed to read CPU online information from file: %s (%s)", __func__,
                    file_name, strerror(errno));
            fclose(file);
            fclose(cpu_file);
            return errno ? -errno : -EIO;
        } else
            fclose(cpu_file);

        if (list != NULL) {
            list[size] = (cpu_usage_t) {
                .name = CPU_LABEL,
                .active = active,
                .total = total,
                .is_online = online
            };
        }

        size++;
    }

    fclose(file);
    return size;
}

static ssize_t get_cooling_devices(thermal_module_t *module, cooling_device_t *list, size_t size) {
    struct stat buffer;
    FILE *file;
    float rpm;
    int ret;

    if (list == NULL)
        return (stat(RPM_FAN_FILE, &buffer) == 0) ? 1 : 0;

    if (size <= 0)
        return -ENOMEM;

    file = fopen(RPM_FAN_FILE, "r");

    if (file == NULL) {
        ALOGE("%s: failed to open: %s", __func__, strerror(errno));
        return -errno;
    }

    ret = fscanf(file, "%f", &rpm);
    fclose(file);

    if (ret != 1)
        return -ENOENT;

    (*list) = (cooling_device_t) {
        .type = FAN_RPM,
        .name = "FAN",
        .current_value = rpm,
    };

    return 1;
}

static struct hw_module_methods_t thermal_module_methods = {
    .open = NULL,
};

thermal_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = THERMAL_HARDWARE_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = THERMAL_HARDWARE_MODULE_ID,
        .name = "Tegra Thermal HAL",
        .author = "The Android Open Source Project",
        .methods = &thermal_module_methods,
    },

    .getTemperatures = get_temperatures,
    .getCpuUsages = get_cpu_usages,
    .getCoolingDevices = get_cooling_devices,
};

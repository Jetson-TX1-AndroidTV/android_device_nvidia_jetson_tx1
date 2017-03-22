# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

common_cflags := \
    -Wa,--noexecstack \
    -Wall \
    -Werror \
    -Wextra \
    -Wformat=2 \
    -Wno-psabi \
    -Wno-unused-parameter \
    -ffunction-sections \
    -fstack-protector-strong \
    -fvisibility=hidden
common_cppflags := \
    -Wnon-virtual-dtor \
    -fno-strict-aliasing \
    -std=gnu++11

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := \
    system/update_engine/nvpayload_update \
    $(LOCAL_PATH)/../include \
    $(LOCAL_PATH)/../include/bct \
    vendor/nvidia/tegra/bootloader/cboot/platform/t210/include \
    $(LOCAL_PATH)/libfdt
LOCAL_SRC_FILES := \
    nv_bootloader_payload_updater.cpp \
    bct/bct.c \
    bct/bct_private.c \
    bct/bct_update.c \
    libfdt/fdt.c \
    libfdt/fdt_ro.c \
    libfdt/fdt_wip.c \
    libfdt/fdt_sw.c \
    libfdt/fdt_rw.c \
    libfdt/fdt_strerror.c \
    libfdt/fdt_util.c
LOCAL_CFLAGS := $(common_cflags)
LOCAL_CFLAGS += -Wno-sign-compare
LOCAL_CPPFLAGS := $(common_cppflags)
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SHARED_LIBRARIES := libchrome
LOCAL_MODULE := libnvblpayload_updater
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := nvpayload_updater_unittest.cpp
LOCAL_MODULE := nvpayload_updater_unittest
LOCAL_C_INCLUDES := \
    system/update_engine/nvpayload_update \
    $(LOCAL_PATH)/../include
LOCAL_STATIC_LIBRARIES := \
    libnvblpayload_updater
LOCAL_SHARED_LIBRARIES := \
    libchrome \
    libhardware
LOCAL_CFLAGS := $(common_cflags)
LOCAL_CFLAGS += -Wno-sign-compare
LOCAL_CPPFLAGS := $(common_cppflags)
include $(BUILD_EXECUTABLE)

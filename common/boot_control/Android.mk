# Copyright (C) 2015 The Android Open Source Project
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

include $(CLEAR_VARS)

LOCAL_MODULE := bootctrl.tegra
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES = $(LOCAL_PATH)/include
LOCAL_SRC_FILES := bootctrl_nvidia.c

LOCAL_SHARED_LIBRARIES := libcutils

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := boot_control.c
LOCAL_SHARED_LIBRARIES := libhardware
LOCAL_MODULE := bootctrl
LOCAL_C_INCLUDES := \
    hardware/libhardware/include \
    $(LOCAL_PATH)/include
LOCAL_CFLAGS := -Wno-unused-parameter

include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))

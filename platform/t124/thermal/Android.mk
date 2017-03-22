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

include $(NVIDIA_DEFAULTS)

LOCAL_C_INCLUDES += device/nvidia/common/thermal
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_STATIC_LIBRARIES := libthermalhal

# Provide thermal sensor info on per-platform basis
ifeq ($(TARGET_PRODUCT),$(filter $(TARGET_PRODUCT),t210ref t210ref_gms t210ref_int))
LOCAL_SRC_FILES += thermal.jetson_cv.c
else
# Report no sensors
LOCAL_SRC_FILES += thermal.default.c
endif

LOCAL_MODULE := thermal.$(TARGET_BOARD_PLATFORM)

include $(NVIDIA_SHARED_LIBRARY)
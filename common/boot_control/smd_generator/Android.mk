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

include $(CLEAR_VARS)

LOCAL_SRC_FILES := generate-smd.c
LOCAL_MODULE := nv_smd_generator
LOCAL_C_INCLUDES := \
    hardware/libhardware/include \
LOCAL_CFLAGS := -Wno-unused-parameter

include $(BUILD_HOST_EXECUTABLE)

include $(NVIDIA_DEFAULTS)

LOCAL_MODULE        := slot_metadata
LOCAL_MODULE_SUFFIX := .bin
LOCAL_MODULE_CLASS  := EXECUTABLES
LOCAL_MODULE_PATH := $(PRODUCT_OUT)

include $(NVIDIA_BASE)
include $(BUILD_SYSTEM)/base_rules.mk
include $(NVIDIA_POST)

$(LOCAL_BUILT_MODULE): PRIVATE_CUSTOM_TOOL = \
	$(ANDROID_HOST_OUT)/bin/nv_smd_generator $(ANDROID_BUILD_TOP)/$@
$(LOCAL_BUILT_MODULE): | nv_smd_generator
	$(transform-generated-source)

.PHONY: $(LOCAL_BUILT_MODULE)

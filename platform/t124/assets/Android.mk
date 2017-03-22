#
# Copyright 2015 The Android Open Source Project
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
#

# This contains the module build definitions for the hardware-specific
# components for this device.
#
# As much as possible, those components should be built unconditionally,
# with device-specific names to avoid collisions, to avoid device-specific
# bitrot and build breakages. Building a component unconditionally does
# *not* include it on all devices, so it is safe even with hardware-specific
# components.
LOCAL_PATH := $(call my-dir)

ifneq ($(wildcard vendor/nvidia/tegra/core-private),)
_nvblob_v2_path := vendor/nvidia/tegra/bootloader/nvbootloader/nvbootutils/nvblob/nvblob_v2
else
_nvblob_v2_path := vendor/nvidia/tegra/prebuilt/t210/host/linux-x86/bin/nvblob_v2
endif

include $(NVIDIA_DEFAULTS)

LOCAL_MODULE        := bmp
LOCAL_MODULE_SUFFIX := .blob
LOCAL_MODULE_CLASS  := EXECUTABLES
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)

include $(NVIDIA_BASE)
include $(BUILD_SYSTEM)/base_rules.mk
include $(NVIDIA_POST)

#
# Invoke script to generate blobs for current product
#
# These direct 3 dependencies make up the command line - order is important!
$(LOCAL_BUILT_MODULE): PRIVATE_CUSTOM_TOOL = $^ $@
$(LOCAL_BUILT_MODULE): \
	$(LOCAL_PATH)/genbmpblob.sh \
	$(LOCAL_PATH)/blob/$(word 1,$(subst _, ,$(TARGET_PRODUCT)))/config_file \
	$(_nvblob_v2_path) \
	$(LOCAL_PATH)/blob_compress.sh \
	$(NVIDIA_LZ4C)

# Dependencies are incomplete - config file lists more files
# To ensure correctness we must mark it as phony target
.PHONY: $(LOCAL_BUILT_MODULE)

$(LOCAL_BUILT_MODULE): | $(ACP)
	$(transform-generated-source)

# clear variables
_nvblob_v2_path :=

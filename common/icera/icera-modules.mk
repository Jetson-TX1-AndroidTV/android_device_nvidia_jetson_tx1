# Copyright (C) 2009 The Android Open Source Project
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

# This lists the packages/files that are necessary to build a device using
# Icera modem

# Phone (in fact: single-flash) or tablet (in fact: dual-flash)?
ifeq ($(NVSI_PRODUCT_CLASS),phone)
NVIDIA_ICERA_TYPE=phone
else
NVIDIA_ICERA_TYPE=tablet
endif

# Deal with proprietary dispatchers
ifneq ($(wildcard vendor/nvidia/tegra/icera/ril-ext-*/dispatcher),)
NVIDIA_PROPRIETARY_DISPATCHER=yes
endif

# APNs
PRODUCT_COPY_FILES += $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/icera/tools/data/etc/apns-conf.xml:system/etc/apns-conf.xml)
PRODUCT_COPY_FILES += $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/icera/tools/data/etc/old-apns-conf.xml:system/etc/old-apns-conf.xml)

# Common settings
PRODUCT_COPY_FILES += $(LOCAL_PATH)/init.icera.common.rc:root/init.icera.common.rc
PRODUCT_PACKAGES += nvidia_tegra_icera_common_modules

# Per-device-type settings (phone or tablet)
PRODUCT_COPY_FILES += $(LOCAL_PATH)/init.icera.$(NVIDIA_ICERA_TYPE).rc:root/init.icera.$(NVIDIA_ICERA_TYPE).rc
PRODUCT_PACKAGES += nvidia_tegra_icera_$(NVIDIA_ICERA_TYPE)_modules

# max retries for Data Connection retry manager - override DEFAULT_MDC_INITIAL_RETRY (1)
PRODUCT_PROPERTY_OVERRIDES += \
    mdc_initial_max_retry=10

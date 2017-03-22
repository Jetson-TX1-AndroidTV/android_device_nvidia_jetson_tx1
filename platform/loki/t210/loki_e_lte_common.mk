# NVIDIA Tegra7 "loki-e" development system
#
# Copyright (c) 2014, NVIDIA Corporation.  All rights reserved.

## This is the file that is common for mp and diag images, for a single sku.

## Common for all loki_e skus
$(call inherit-product, $(LOCAL_PATH)/loki_e_common.mk)

## Icera modem integration
# APNs for data-only devices - adding rule below so it takes precedence over that from icera-modules.mk
PRODUCT_COPY_FILES += $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/icera/tools/data/etc/apns-conf-data-only.xml:system/etc/apns-conf.xml)
$(call inherit-product-if-exists, $(LOCAL_PATH)/../../../common/icera/icera-modules.mk)
$(call inherit-product-if-exists, $(LOCAL_PATH)/../../../common/icera/firmware/nvidia-e1729-loki/fw-cpy-nvidia-e1729-loki-do-prod.mk)
PRODUCT_COPY_FILES += \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_PATH)/init.icera.rc:root/init.icera.rc) \
        $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/icera/ril/icera-util/ril_atc.usb.config:system/etc/ril_atc.config)
PRODUCT_PROPERTY_OVERRIDES += \
    ro.modem.do=1


PRODUCT_PACKAGE_OVERLAYS += $(LOCAL_PATH)/../overlays/do

## factory script
ifeq ($(wildcard vendor/nvidia/tegra/apps/diagsuite),vendor/nvidia/tegra/apps/diagsuite)
PRODUCT_COPY_FILES += \
    vendor/nvidia/tegra/apps/diagsuite/bin/release/flags/flag_for_loki_e_lte.txt:flag_for_loki_e_lte.txt
endif

$(call inherit-product, $(LOCAL_PATH)/../loki_common.mk)


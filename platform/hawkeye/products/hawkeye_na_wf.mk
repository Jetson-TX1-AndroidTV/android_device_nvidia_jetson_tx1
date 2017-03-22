# NVIDIA Tegra7 "hawkeye" development system
#
# Copyright (c) 2014-2016, NVIDIA Corporation.  All rights reserved.
#
include $(LOCAL_PATH)/hawkeye_base.mk

## Thse are default settings, it gets changed as per sku manifest properties
PRODUCT_NAME := hawkeye_na_wf
PRODUCT_MODEL := hawkeye_na_wf

## Value of PRODUCT_NAME is mangeled before it can be
## used because of call to inherits, store their values to
## use later in this file below
_product_name := $(strip $(PRODUCT_NAME))

## Copy gpsconfig file
PRODUCT_COPY_FILES += \
   $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/3rdparty/broadcom/gps/bin/gpsconfig-wf-t210ref.xml:system/etc/gps/gpsconfig.xml)

## SKU specific overrides
PRODUCT_PROPERTY_OVERRIDES += ro.radio.noril=true
## GPS configuration
DEVICE_PACKAGE_OVERLAYS := \
    $(LOCAL_PATH)/../../../product/tablet/overlay-tablet/$(PLATFORM_VERSION_LETTER_CODE) \
    $(DEVICE_PACKAGE_OVERLAYS)

# This flag is required in order to differentiate between platforms that use
# Keymaster1.0 vs the legacy keymaster 0.3 service.
USES_KEYMASTER_1 := true

# This flag indicates that this platform uses a TLK based Gatekeeper.
USES_GATEKEEPER := true

## nvidia apps for this sku
$(call inherit-product-if-exists, $(_product_private_path)/$(_product_name).mk)

## factory ramdisk
$(call inherit-product-if-exists, vendor/nvidia/tegra/apps/factory/factory-ramdisk/device/nvidia/platform/hawkeye/hawkeye.mk)

## Clean local variables
_product_name :=
_product_private_path :=

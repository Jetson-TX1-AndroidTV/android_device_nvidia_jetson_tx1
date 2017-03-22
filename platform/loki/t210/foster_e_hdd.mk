# NVIDIA Tegra7 "foster-e-hdd" development system
#
# Copyright (c) 2014-2016, NVIDIA Corporation.  All rights reserved.

## All essential packages
$(call inherit-product, device/nvidia/soc/t210/device-t210.mk)
$(call inherit-product, device/nvidia/product/tv/device.mk)
$(call inherit-product, device/nvidia/platform/t210/device.mk)

## Install GMS if available
$(call inherit-product-if-exists, 3rdparty/google/gms-apps/tv/64/products/gms.mk)
PRODUCT_PROPERTY_OVERRIDES += \
        ro.com.google.clientidbase=android-nvidia

## Thse are default settings, it gets changed as per sku manifest properties
PRODUCT_NAME := foster_e_hdd
PRODUCT_DEVICE := t210
PRODUCT_MODEL := foster_e_hdd
PRODUCT_MANUFACTURER := NVIDIA
PRODUCT_BRAND := nvidia

PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=320

PRODUCT_PACKAGES += \
	toolC \
	smartctl

PRODUCT_PACKAGES += \
    PlexMediaServer \
    NvXtraMediaV \
    NvIgnition

#SHIELD user registration
PRODUCT_PACKAGES += \
    NvRegistration

## Values of PRODUCT_NAME and PRODUCT_DEVICE are mangeled before it can be
## used because of call to inherits, store their values to use later in this
## file below
_product_name := $(strip $(PRODUCT_NAME))
_product_device := $(strip $(PRODUCT_DEVICE))

## Copy gpsconfig file
PRODUCT_COPY_FILES += \
    $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/3rdparty/broadcom/gps/bin/gpsconfig-wf-t210ref.xml:system/etc/gps/gpsconfig.xml)

## common for mp and diag images, for a single sku.
$(call inherit-product, $(LOCAL_PATH)/foster_e_hdd_common.mk)

## Factory scripts, common for mp images, among multiple skus.
$(call inherit-product-if-exists, vendor/nvidia/diag/common/mp_common.mk)

## common apps for all skus
$(call inherit-product-if-exists, vendor/nvidia/loki/skus/$(_product_name).mk)

## nvidia apps for this sku
$(call inherit-product-if-exists, vendor/nvidia/$(_product_device)/skus/$(_product_name).mk)

## 3rd-party apps for this sku
$(call inherit-product-if-exists, 3rdparty/applications/prebuilt/common/$(_product_name).mk)
$(call inherit-product-if-exists, vendor/nvidia/loki/skus/tegrazone_next.mk)

## eks2 data blob
PRODUCT_COPY_FILES += \
    $(call add-to-product-copy-files-if-exists, device/nvidia/platform/t210/eks2/eks2_foster.dat:vendor/app/eks2/eks2.dat)

## Clean local variables
_product_name :=
_product_device :=


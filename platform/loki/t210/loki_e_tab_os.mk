# NVIDIA Tegra7 "loki-e" development system
#
# Copyright (c) 2014, NVIDIA Corporation.  All rights reserved.
#
# AndroidProducts.mk is included before BoardConfig.mk, variable essential at
# start of build and used in here should always be intialized in this file

## All essential packages
$(call inherit-product, device/nvidia/soc/t210/device-t210.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)
$(call inherit-product, device/nvidia/platform/t210/device.mk)
$(call inherit-product, device/nvidia/product/tablet/device.mk)

## Install GMS if available
TARGET_GMS_TABLET_ARCH := arm64
$(call inherit-product-if-exists, 3rdparty/google/gms-apps/tablet/products/gms.mk)
PRODUCT_PROPERTY_OVERRIDES += \
        ro.com.google.clientidbase=android-nvidia

# Sensor package definition
SENSOR_BUILD_VERSION		:= default
SENSOR_HAL_API			:= 1.4
SENSOR_HAL_VERSION		:= nvs
HAL_OS_INTERFACE		:= NvsAos.cpp
SENSOR_FUSION_VENDOR		:= Invensense
SENSOR_FUSION_VERSION		:= mpl530
SENSOR_FUSION_BUILD_DIR		:= mpl530.nvs
PRODUCT_PROPERTY_OVERRIDES += \
    ro.hardware.sensors=$(SENSOR_BUILD_VERSION).api_v$(SENSOR_HAL_API).$(SENSOR_FUSION_VERSION).$(SENSOR_HAL_VERSION)
PRODUCT_PACKAGES += \
    sensors.$(SENSOR_BUILD_VERSION).api_v$(SENSOR_HAL_API).$(SENSOR_FUSION_VERSION).$(SENSOR_HAL_VERSION)

## Thse are default settings, it gets changed as per sku manifest properties
PRODUCT_NAME := loki_e_tab_os
PRODUCT_DEVICE := t210
PRODUCT_MODEL := loki_e_tab_os
PRODUCT_MANUFACTURER := NVIDIA
PRODUCT_BRAND := nvidia


## Values of PRODUCT_NAME and PRODUCT_DEVICE are mangeled before it can be
## used because of call to inherits, store their values to
## use later in this file below
_product_name := $(strip $(PRODUCT_NAME))
_product_device := $(strip $(PRODUCT_DEVICE))

## common for mp and diag images, for a single sku.
$(call inherit-product, $(LOCAL_PATH)/loki_e_tab_os_common.mk)

## Factory scripts, common for mp images, among multiple skus.
$(call inherit-product-if-exists, vendor/nvidia/diag/common/mp_common.mk)

## common apps for all skus
$(call inherit-product-if-exists, vendor/nvidia/loki/skus/tab_os_common.mk)

## nvidia apps for this sku
$(call inherit-product-if-exists, vendor/nvidia/$(_product_device)/skus/$(_product_name).mk)

## 3rd-party apps for this sku
$(call inherit-product-if-exists, 3rdparty/applications/prebuilt/common/$(_product_name).mk)

## Place here sku-specific and mp-only code.

## Clean local variables
_product_name :=
_product_device :=


# NVIDIA Tegra6 "T132ref" development system
#
# Copyright (c) 2013-2015 NVIDIA Corporation.  All rights reserved.

$(call inherit-product, device/nvidia/soc/t210/device-t210.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)
$(call inherit-product, device/nvidia/platform/t210/device.mk)
$(call inherit-product, device/nvidia/product/tablet/device.mk)

## Verified_boot
$(call inherit-product,build/target/product/verity.mk)

PRODUCT_NAME := t210ref
PRODUCT_DEVICE := t210
PRODUCT_MODEL := t210ref
PRODUCT_MANUFACTURER := NVIDIA
PRODUCT_BRAND := nvidia

ifeq ($(PLATFORM_IS_AFTER_M),)
HOST_PREFER_32_BIT := true
endif

TARGET_SYSTEM_PROP    += device/nvidia/platform/t210/t210ref.prop

## Values of PRODUCT_NAME and PRODUCT_DEVICE are mangeled before it can be
## used because of call to inherits, store their values to
## use later in this file below
_product_name := $(strip $(PRODUCT_NAME))
_product_device := $(strip $(PRODUCT_DEVICE))

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/../../common/init.cal.rc:root/init.cal.rc \
    $(LOCAL_PATH)/gpio-keys.kl:system/usr/keylayout/gpio-keys.kl \
    $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/3rdparty/broadcom/gps/bin/gpsconfig-wf-t210ref.xml:system/etc/gps/gpsconfig.xml) \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.software.midi.xml:system/etc/permissions/android.software.midi.xml \
    device/nvidia/platform/t210/ussrd.conf:system/etc/ussrd.conf

PRODUCT_AAPT_CONFIG += xlarge large

## Verified_boot
PRODUCT_SYSTEM_VERITY_PARTITION := /dev/block/platform/sdhci-tegra.3/by-name/APP
PRODUCT_VENDOR_VERITY_PARTITION := /dev/block/platform/sdhci-tegra.3/by-name/vendor

# Common for all ers
$(call inherit-product, $(LOCAL_PATH)/ers_common.mk)

## SKU specific overrides
PRODUCT_PROPERTY_OVERRIDES += ro.radio.noril=true

## Tablet configuration
DEVICE_PACKAGE_OVERLAYS += device/nvidia/product/tablet/overlay-tablet/$(PLATFORM_VERSION_LETTER_CODE)

## Sensor package definition
SENSOR_BUILD_VERSION    	:= default
SENSOR_HAL_API          	:= 1.4
SENSOR_HAL_VERSION      	:= nvs
HAL_OS_INTERFACE        	:= NvsAos.cpp
SENSOR_FUSION_VENDOR		:= Invensense
SENSOR_FUSION_VERSION   	:= mpl530
SENSOR_FUSION_BUILD_DIR		:= mpl530.nvs
PRODUCT_PROPERTY_OVERRIDES += \
    ro.hardware.sensors=$(SENSOR_BUILD_VERSION).api_v$(SENSOR_HAL_API).$(SENSOR_FUSION_VERSION).$(SENSOR_HAL_VERSION)
PRODUCT_PACKAGES += \
    sensors.$(SENSOR_BUILD_VERSION).api_v$(SENSOR_HAL_API).$(SENSOR_FUSION_VERSION).$(SENSOR_HAL_VERSION)

#Thermal HALs
PRODUCT_PACKAGES += \
    thermal.tegra

PRODUCT_PACKAGES += \
    rp3_image

# for warning
PRODUCT_PACKAGES += \
    slideshow \
    verity_warning_images \
    libnvjni_tinyplanet \
    libnvjni_jpegutil \
    libcom_nvidia_nvcamera_util_NativeUtils \
    libjni_nvmosaic \
    libnvraw_creator


# for v4l2 test
PRODUCT_PACKAGES += \
    v4l2-ctl \
    v4l2-compliance \
    v4l2-dbg \
    media-ctl

#symlinks
PRODUCT_PACKAGES += \
    camera.jnilib1.symlink \
    camera.jnilib2.symlink \
    camera.jnilib3.symlink \
    camera.jnilib4.symlink \
    camera.jnilib5.symlink

## Factory Reset Protection to be disabled in RP2 Partition
PRODUCT_COPY_FILES += device/nvidia/tegraflash/t210/rp2_binaries/rp2_disable_frp.bin:rp2.bin

## Verified_boot
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.verified_boot.xml:system/etc/permissions/android.software.verified_boot.xml

## common apps for all skus
$(call inherit-product-if-exists, vendor/nvidia/$(_product_device)/skus/t210ref_variants_common.mk)

## nvidia apps for this sku
$(call inherit-product-if-exists, vendor/nvidia/$(_product_device)/skus/$(_product_name).mk)

ifeq ($(NV_ANDROID_FRAMEWORK_ENHANCEMENTS),TRUE)
PRODUCT_PACKAGE_OVERLAYS += vendor/nvidia/jetson/overlays/common
endif

## Calibration notifier
PRODUCT_PACKAGES += CalibNotifier
PRODUCT_COPY_FILES += \
    device/nvidia/platform/t210/calibration/calib_cfg.xml:system/etc/calib_cfg.xml

# Launcher3
PRODUCT_PACKAGES += Launcher3

# FW check
LOCAL_FW_CHECK_TOOL_PATH=device/nvidia/common/fwcheck
LOCAL_FW_XML_PATH=vendor/nvidia/t210/skus
PRODUCT_COPY_FILES += $(call add-to-product-copy-files-if-exists, $(LOCAL_FW_XML_PATH)/fw_version.xml:$(TARGET_COPY_OUT_VENDOR)/etc/fw_version.xml) \
    $(call add-to-product-copy-files-if-exists, $(LOCAL_FW_CHECK_TOOL_PATH)/fw_check.py:fw_check.py)

# This flag is required in order to differentiate between platforms that use
# Keymaster1.0 vs the legacy keymaster 0.2 service.
USES_KEYMASTER_1 := true

# This flag indicates that this platform uses a TLK based Gatekeeper.
USES_GATEKEEPER := true

#This flag indicates vrr/rsa support
USES_GS_RSA_KEYS := false

# Include ShieldTech
SHIELDTECH_FEATURE_NVLAUNCHER := false
SHIELDTECH_FEATURE_KEYBOARD := false
SHIELDTECH_FEATURE_NVGALLERY := false
SHIELDTECH_FEATURE_CONSOLE_MODE := false
SHIELDTECH_FEATURE_OSC := false
SHIELDTECH_FEATURE_BLAKE := true
$(call inherit-product-if-exists, vendor/nvidia/shieldtech/common/shieldtech.mk)

## Clean local variables
_product_name :=
_product_device :=


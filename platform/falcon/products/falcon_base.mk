# Copyright (c) 2014-2016, NVIDIA Corporation.  All rights reserved.
#
#  common makefile
#

## All essential packages
$(call inherit-product, device/nvidia/soc/t210/device-t210.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)
$(call inherit-product, device/nvidia/platform/t210/device.mk)
$(call inherit-product, device/nvidia/product/tablet/device.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)
## Verified_boot
$(call inherit-product,build/target/product/verity.mk)
## Enable fscheck
$(call inherit-product, device/nvidia/common/fscheck.mk)

## Thse are default settings, it gets changed as per sku manifest properties
PRODUCT_DEVICE := t210
PRODUCT_MANUFACTURER := NVIDIA
PRODUCT_BRAND := nvidia

APPFILTERCLASS := com.nvidia.appfilter

# WAR for android M
ifneq ($(PLATFORM_IS_AFTER_M),1)
HOST_PREFER_32_BIT := true
endif

WITH_DEXPREOPT := false
DONT_DEXPREOPT_PREBUILTS := true

# _product_private_path is cleared in falcon_ product makefiles
_product_private_path := vendor/nvidia/falcon

PRODUCT_AAPT_CONFIG += mdpi hdpi xhdpi sw320dp sw340dp sw360dp sw380dp sw540dp sw600dp small normal large xlarge

## Verified_boot
PRODUCT_SYSTEM_VERITY_PARTITION := /dev/block/platform/sdhci-tegra.3/by-name/APP
PRODUCT_VENDOR_VERITY_PARTITION := /dev/block/platform/sdhci-tegra.3/by-name/vendor

# for warning
PRODUCT_PACKAGES += \
    slideshow \
    verity_warning_images

## using customized sound effect
$(call inherit-product-if-exists, $(_product_private_path)/media/audio/AudioPackage.mk)

## Supported locale
PRODUCT_LOCALES := en_US in_ID ca_ES cs_CZ da_DK de_DE en_GB es_ES es_US tl_PH fr_FR hr_HR it_IT lv_LV lt_LT hu_HU nl_NL nb_NO pl_PL pt_BR pt_PT ro_RO sk_SK sl_SI fi_FI sv_SE vi_VN tr_TR el_GR bg_BG ru_RU sr_RS uk_UA iw_IL ar_EG fa_IR th_TH ko_KR zh_CN zh_TW ja_JP

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/../../../common/init.cal.rc:root/init.cal.rc \
    $(LOCAL_PATH)/../../../common/init.ndiscovery.rc:root/init.ndiscovery.rc \
    frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.software.midi.xml:system/etc/permissions/android.software.midi.xml \
    frameworks/native/data/etc/android.software.freeform_window_management.xml:system/etc/permissions/android.software.freeform_window_management.xml \
    frameworks/native/data/etc/android.software.picture_in_picture.xml:system/etc/permissions/android.software.picture_in_picture.xml \
    $(LOCAL_PATH)/media_profiles.xml:system/etc/media_profiles.xml \
    $(LOCAL_PATH)/nvcamera.conf:system/etc/nvcamera.conf \
    device/nvidia/platform/falcon/nvaudio_conf.xml:system/etc/nvaudio_conf.xml \
    device/nvidia/platform/falcon/nvaudio_conf_pdm1.xml:system/etc/nvaudio_conf_pdm1.xml \
    device/nvidia/platform/falcon/nvaudio_factory_conf.xml:system/etc/nvaudio_factory_conf.xml \
    device/nvidia/platform/falcon/nvaudio_factory_conf_pdm1.xml:system/etc/nvaudio_factory_conf_pdm1.xml \
    device/nvidia/platform/falcon/nvaudio_fx.xml:system/etc/nvaudio_fx.xml \
    device/nvidia/platform/falcon/audio_effects.conf:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.conf \
    device/nvidia/platform/t210/ussrd.conf:system/etc/ussrd.conf \
    $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/core-private/utils/cyload/cyupdate.sh:$(TARGET_COPY_OUT_VENDOR)/bin/cyupdate.sh) \
    $(call add-to-product-copy-files-if-exists, vendor/nvidia/falcon/utils/cyload/cyupdate_config.sh:$(TARGET_COPY_OUT_VENDOR)/bin/cyupdate_config.sh)

## Verified_boot
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.verified_boot.xml:system/etc/permissions/android.software.verified_boot.xml

## Sensor package definition
SENSOR_BUILD_VERSION    := default
SENSOR_HAL_API          := 1.4
SENSOR_HAL_VERSION      := nvs
HAL_OS_INTERFACE        := NvsAos.cpp
SENSOR_FUSION_VENDOR	:= Invensense
SENSOR_FUSION_VERSION   := mpl530
SENSOR_FUSION_BUILD_DIR	:= mpl530.nvs
PRODUCT_PROPERTY_OVERRIDES += \
    ro.hardware.sensors=$(SENSOR_BUILD_VERSION).api_v$(SENSOR_HAL_API).$(SENSOR_FUSION_VERSION).$(SENSOR_HAL_VERSION)
PRODUCT_PACKAGES += \
    sensors.$(SENSOR_BUILD_VERSION).api_v$(SENSOR_HAL_API).$(SENSOR_FUSION_VERSION).$(SENSOR_HAL_VERSION)

## Key Layouts
PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/Vendor_0955_Product_7212.kl:system/usr/keylayout/Vendor_0955_Product_7212.kl

## NVSS using the Accelerometer to detect if back mic is obstructed (tablet seating flat on table)
PRODUCT_PACKAGES += NVSS
PRODUCT_TARGET_HAS_NVSS_ENABLED := yes

ifeq ($(NV_ANDROID_FRAMEWORK_ENHANCEMENTS_AUDIO),TRUE)
PRODUCT_COPY_FILES += \
    device/nvidia/platform/falcon/audio_policy.conf:system/etc/audio_policy.conf
else
PRODUCT_COPY_FILES += \
    device/nvidia/platform/falcon/audio_policy_noenhance.conf:system/etc/audio_policy.conf
endif

## Camera permission file
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.full.xml:system/etc/permissions/android.hardware.camera.full.xml \
    frameworks/native/data/etc/android.hardware.camera.raw.xml:system/etc/permissions/android.hardware.camera.raw.xml

# RP2 partition binary to enable Factory Reset Protection
PRODUCT_COPY_FILES += device/nvidia/tegraflash/t210/rp2_binaries/rp2_enable_frp.bin:rp2.bin

# RP3 partiton for wifi country code
PRODUCT_PACKAGES += \
    rp3_image

## default assets file
PRODUCT_COPY_FILES += \
    device/nvidia/platform/falcon/products/bitmap/charged.png:root/res/images/charger/charged.png \
    device/nvidia/platform/falcon/products/bitmap/charging.png:root/res/images/charger/charging.png \
    device/nvidia/platform/falcon/products/bitmap/fullycharged.png:root/res/images/charger/fullycharged.png \
    device/nvidia/platform/falcon/products/bitmap/lowbat.png:root/res/images/charger/lowbat.png

TARGET_SYSTEM_PROP    += device/nvidia/platform/falcon/system.prop

# Empty UDA image
PRODUCT_COPY_FILES += \
    device/nvidia/platform/falcon/products/empty_userdata_32g.img:userdata32.img

$(call inherit-product-if-exists, vendor/nvidia/tegra/core/android/services/analyzer.mk)

# FW check
LOCAL_FW_CHECK_TOOL_PATH=device/nvidia/common/fwcheck
LOCAL_FW_XML_PATH=vendor/nvidia/falcon/sku
PRODUCT_COPY_FILES += $(call add-to-product-copy-files-if-exists, $(LOCAL_FW_XML_PATH)/fw_version.xml:$(TARGET_COPY_OUT_VENDOR)/etc/fw_version.xml) \
    $(call add-to-product-copy-files-if-exists, $(LOCAL_FW_CHECK_TOOL_PATH)/fw_check.py:fw_check.py)

# Wi-Fi country code system properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.factory.wifi=/mnt/factory/wifi \
    ro.factory.wifi.lbs=true

# Disable fs selection feature for dev build
ifneq ($(TARGET_BUILD_VARIANT),eng)
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.fscheck.device=/dev/block/platform/sdhci-tegra.3/by-name/RP4 \
    ro.fscheck.mode=auto \
    ro.fscheck.enable=1 \
    ro.fscheck.def_fstype=ext4

endif

## factory ramdisk
 $(call inherit-product-if-exists, vendor/nvidia/tegra/apps/factory/uda_diag/device_nvidia/platform/falcon/products/falcon.mk)


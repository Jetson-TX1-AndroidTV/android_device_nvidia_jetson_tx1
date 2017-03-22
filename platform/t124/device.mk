# NVIDIA Tegra6 "T132" development system
#
# Copyright (c) 2013-2016 NVIDIA Corporation.  All rights reserved.
#
# 32-bit specific product settings

$(call inherit-product-if-exists, frameworks/base/data/videos/VideoPackage2.mk)
$(call inherit-product, device/nvidia/platform/t210/device-common.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/t210/nvflash.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/touch/raydium.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/touch/sharp.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/touch/nvtouch.mk)
$(call inherit-product, device/nvidia/platform/t210/motionq/motionq.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/multimedia/base.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/multimedia/firmware.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/camera/full.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/services/nvcpl.mk)
$(call inherit-product-if-exists, vendor/nvidia/tegra/core/android/services/analyzer.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/services/edid.mk)

#enable Widevine drm
PRODUCT_PROPERTY_OVERRIDES += drm.service.enabled=true
PRODUCT_PACKAGES += \
    com.google.widevine.software.drm.xml \
    com.google.widevine.software.drm \
    libdrmwvmplugin \
    libwvm \
    libWVStreamControlAPI_L1 \
    libwvdrm_L1

PRODUCT_COPY_FILES += \
   device/nvidia/platform/loki/t210/pbc.conf:system/etc/pbc.conf

PRODUCT_PACKAGES += \
    bpmp \
    tegra_xusb_firmware \
    tegra21x_xusb_firmware

PRODUCT_PACKAGES += \
        tos \
        keystore.tegra \
        gatekeeper.tegra \
        icera_host_test \
        setup_fs \
        e2fsck \
        make_ext4fs \
        hdmi_cec.tegra \
        lights.tegra \
        pbc.tegra \
        pbc.hawkeye \
        power.tegra \
        power.loki_e \
        power.loki_e_lte \
        power.loki_e_wifi \
        power.darcy \
        power.foster_e \
        power.foster_e_hdd \
        libnvglsi \
        libnvwsi

# HDCP SRM Support
PRODUCT_PACKAGES += \
        hdcp1x.srm \
        hdcp2x.srm \
        hdcp2xtest.srm
#dual wifi
PRODUCT_PACKAGES += \
    libnvwifi-service \
    wpa_supplicant_2
PRODUCT_COPY_FILES += \
   device/nvidia/drivers/comms/brcm_wpa_xlan.conf:system/etc/firmware/brcm_wpa_xlan.conf

#enable Widevine drm
PRODUCT_PROPERTY_OVERRIDES += drm.service.enabled=true
PRODUCT_PACKAGES += \
    liboemcrypto \
    libdrmdecrypt

PRODUCT_RUNTIMES := runtime_libart_default

PRODUCT_PACKAGES += \
    gpload \
    ctload \
    c2debugger

#TegraOTA
PRODUCT_PACKAGES += \
    TegraOTA

ifneq ($(wildcard vendor/nvidia/tegra/core-private),)
PRODUCT_PACKAGES += \
    track.sh
endif

ifeq ($(NV_ANDROID_FRAMEWORK_ENHANCEMENTS),TRUE)
PRODUCT_PACKAGES += \
    Stats
endif

# LED service
PRODUCT_PACKAGES += \
    NvShieldService \
    NvGamepadMonitorService

# Application for sending feedback to NVIDIA
PRODUCT_PACKAGES += \
    nvidiafeedback

# Paragon filesystem solution binaries
PRODUCT_PACKAGES += \
    mountufsd \
    chkufsd \
    mkexfat \
    chkexfat \
    mkhfs \
    chkhfs \
    mkntfs \
    chkntfs

# Leanback Launcher
PRODUCT_PACKAGES += \
    AppDrawer \
    LeanbackLauncher \
    LeanbackCustomizer \
    LeanbackIme \
    TvProvider \
    TvSettings \
    tv_input.default \
    TV

# Leanback Gapps
$(call inherit-product, vendor/google/jetson/jetson-vendor.mk)
$(call inherit-product-if-exists, vendor/google/atv/atv-vendor.mk)

#for SMD partition
PRODUCT_PACKAGES += \
    slot_metadata

# touch screen and camera don't apply to Foster/Darcy
ifeq ($(filter foster_e% darcy%, $(TARGET_PRODUCT)),)
# Sharp touch
PRODUCT_COPY_FILES += \
    device/nvidia/drivers/touchscreen/lr388k7_ts.idc:system/usr/idc/lr388k7_ts.idc \
    device/nvidia/common/init.sharp_touch.rc:root/init.sharp_touch.rc

# Nvidia touch
PRODUCT_COPY_FILES += \
    device/nvidia/common/init.nv_touch.rc:root/init.nv_touch.rc

# Nvidia Camera app
PRODUCT_PACKAGES += NvCamera
endif

# bmp.blob
PRODUCT_PACKAGES += bmp

#symlinks
PRODUCT_PACKAGES += gps.symlink

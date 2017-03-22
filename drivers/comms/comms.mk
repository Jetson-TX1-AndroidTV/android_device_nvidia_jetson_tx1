#
# Copyright (c) 2015 NVIDIA Corporation.  All Rights Reserved.
#
# NVIDIA Corporation and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA Corporation is strictly prohibited.
#

PRODUCT_PACKAGES += \
    hostapd \
    wpa_supplicant \
    wpa_supplicant.conf

PRODUCT_COPY_FILES += \
  frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
  frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
  frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
  frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml \
  frameworks/native/data/etc/android.hardware.location.xml:system/etc/permissions/android.hardware.location.xml \
  device/nvidia/common/wifi_loader.sh:system/bin/wifi_loader.sh \
  device/nvidia/common/bt_loader.sh:system/bin/bt_loader.sh \
  device/nvidia/common/wpa_supplicant.sh:system/bin/wpa_supplicant.sh \
  device/nvidia/common/supplicant_log_monitor.sh:system/bin/supplicant_log_monitor.sh

PRODUCT_COPY_FILES += \
    device/nvidia/common/init.comms.rc:root/init.comms.rc \
    device/nvidia/common/gps_select.sh:system/bin/gps_select.sh

PRODUCT_COPY_FILES += \
    $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/3rdparty/broadcom/gps/bin/gps.conf:system/etc/gps.conf)

# Copy wpa/p2p ovelay config files
PRODUCT_COPY_FILES += \
    hardware/broadcom/wlan/bcmdhd/config/wpa_supplicant_overlay.conf:system/etc/wifi/wpa_supplicant_overlay.conf \
    hardware/broadcom/wlan/bcmdhd/config/p2p_supplicant_overlay.conf:system/etc/wifi/p2p_supplicant_overlay.conf \
    device/nvidia/common/wifi_scan_config.conf:system/etc/wifi/wifi_scan_config.conf

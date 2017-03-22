# NVIDIA Tegra7 "hawkeye" development system
#
# Copyright (c) 2014-2016, NVIDIA Corporation.  All rights reserved.
#

include $(LOCAL_PATH)/hawkeye_base.mk

## including rild here to create modem for data only skus without dialer and
## mms apps , not including generic.mk
PRODUCT_PACKAGES += rild

## Thse are default settings, it gets changed as per sku manifest properties
PRODUCT_NAME := hawkeye_un_do
PRODUCT_MODEL := hawkeye_un_do

## Value of PRODUCT_NAME is mangeled before it can be
## used because of call to inherits, store their values to
## use later in this file below
_product_name := $(strip $(PRODUCT_NAME))

## Copy gpsconfig file
PRODUCT_COPY_FILES += \
    $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/3rdparty/broadcom/gps/bin/gpsconfig-t210ref.xml:system/etc/gps/gpsconfig.xml)

## Icera modem integration

# APNs for data-only devices - adding rule below so it takes precedence over that from icera-modules.mk
PRODUCT_COPY_FILES += \
        $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/icera/tools/data/etc/apns-conf-data-only.xml:system/etc/apns-conf.xml) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_PATH)/../init.icera.rc:root/init.icera.rc) \
        $(call add-to-product-copy-files-if-exists, vendor/nvidia/tegra/icera/ril/icera-util/ril_atc.usb.config:system/etc/ril_atc.config)

$(call inherit-product-if-exists, $(LOCAL_PATH)/../../../common/icera/icera-modules.mk)
$(call inherit-product-if-exists, $(LOCAL_PATH)/../../../common/icera/firmware/nvidia-e1729-hawkeye/fw-cpy-nvidia-e1729-hawkeye-do-prod.mk)

PRODUCT_PROPERTY_OVERRIDES += ro.modem.do=1

# notifier:OFF --> disable modem notification of wifi access point events
# datastall:ON --> enable the dynamic datastall timer configuration, the APN name to take into
# consideration should be given in the property
PRODUCT_PROPERTY_OVERRIDES += ril.icera-config-args=notifier:OFF,datastall:ON,lwaactivate

PRODUCT_PACKAGES += icera-config

## SKU specific overrides
include frameworks/native/build/phone-xhdpi-2048-dalvik-heap.mk
DEVICE_PACKAGE_OVERLAYS := \
    $(LOCAL_PATH)/../../../product/tablet/overlay-tablet-do/$(PLATFORM_VERSION_LETTER_CODE) \
    $(DEVICE_PACKAGE_OVERLAYS)

## nvidia apps for this sku
$(call inherit-product-if-exists, $(_product_private_path)/$(_product_name).mk)

## factory ramdisk
$(call inherit-product-if-exists, vendor/nvidia/tegra/apps/factory/factory-ramdisk/device/nvidia/platform/hawkeye/hawkeye.mk)

## Clean local variables
_product_name :=
_product_private_path :=

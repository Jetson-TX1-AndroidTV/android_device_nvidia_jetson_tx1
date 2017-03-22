# NVIDIA Tegra7 "falcon" development system
#
# Copyright (c) 2016, NVIDIA Corporation.  All rights reserved.
#

## All essential packages
$(call inherit-product, device/nvidia/soc/t210/device-t210.mk)
$(call inherit-product-if-exists, vendor/nvidia/falcon/media/audio/AudioPackage.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)
$(call inherit-product, device/nvidia/platform/t210/device.mk)
$(call inherit-product, device/nvidia/product/tablet/device.mk)

include $(LOCAL_PATH)/falcon_base.mk

## Thse are default settings, it gets changed as per sku manifest properties
PRODUCT_DEVICE := falcon
PRODUCT_NAME := falcon_na_wf
PRODUCT_MODEL := falcon_na_wf

_product_private_path := vendor/nvidia/falcon

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
DEVICE_PACKAGE_OVERLAYS += device/nvidia/product/tablet/overlay-tablet/$(PLATFORM_VERSION_LETTER_CODE)

# This flag is required in order to differentiate between platforms that use
# Keymaster1.0 vs the legacy keymaster 0.3 service.
USES_KEYMASTER_1 := true

# This flag indicates that this platform uses a TLK based Gatekeeper.
USES_GATEKEEPER := true

# Enable A/B system update
AB_OTA_UPDATER :=true
AB_OTA_PARTITIONS := \
	boot \
	system \
	vendor \
	recovery

PRODUCT_PACKAGES += \
	update_engine \
	update_engine_client \
	update_verifier \
	bootctrl.tegra \
	brillo_update_payload

#For AB_OTA debug purpose
ifneq ($(TARGET_BUILD_VARIANT), user)
PRODUCT_PACKAGES += \
	bootctrl
endif

## nvidia apps for this sku
$(call inherit-product-if-exists, $(_product_private_path)/$(_product_name).mk)

## Clean local variables
_product_name :=
_product_private_path :=

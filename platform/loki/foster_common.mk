# NVIDIA Tegra4 "foster" development system
#
# Copyright (c) 2013-2015 NVIDIA Corporation.  All rights reserved.
#

PRODUCT_LOCALES := en_US in_ID ca_ES cs_CZ da_DK de_DE en_GB es_ES es_US tl_PH fr_FR hr_HR it_IT lv_LV lt_LT hu_HU nl_NL nb_NO pl_PL pt_BR pt_PT ro_RO sk_SK sl_SI fi_FI sv_SE vi_VN tr_TR el_GR bg_BG ru_RU sr_RS uk_UA iw_IL ar_EG fa_IR th_TH ko_KR zh_CN zh_TW ja_JP

PRODUCT_PROPERTY_OVERRIDES += ro.radio.noril=true

PRODUCT_PACKAGE_OVERLAYS += $(LOCAL_PATH)/overlays/wifi

DEVICE_PACKAGE_OVERLAYS := $(LOCAL_PATH)/overlays/common

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=320

# Additional AOSP packages not included in Android TV
PRODUCT_PACKAGES += \
    DocumentsUI \
    cyload \
    CaptivePortalLogin

PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/Vendor_0955_Product_7205.kl:system/usr/keylayout/Vendor_0955_Product_7205.kl \
  $(LOCAL_PATH)/gpio-keys.kl:system/usr/keylayout/gpio-keys.kl \
  device/nvidia/platform/t210/ussrd.foster.conf:system/etc/ussrd.conf \
  $(call add-to-product-copy-files-if-exists, vendor/nvidia/loki/utils/cyload/cyupdate.sh:$(TARGET_COPY_OUT_VENDOR)/bin/cyupdate.sh)


PRODUCT_AAPT_CONFIG += xlarge large

## Common packages
$(call inherit-product-if-exists, vendor/nvidia/tegra/core/android/services/analyzer.mk)

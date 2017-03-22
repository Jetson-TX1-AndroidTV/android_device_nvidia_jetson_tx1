# Copyright (c) 2014 NVIDIA Corporation.  All rights reserved.

SYSTEM_ICERA_FW_PATH=$(TARGET_COPY_OUT_VENDOR)/firmware/icera
LOCAL_ICERA_FW_PATH_ROOT=vendor/nvidia/tegra/icera/firmware/binaries/binaries_nvidia-e1729-tn8l

# Is there a dev folder?
ifneq ($(wildcard $(LOCAL_ICERA_FW_PATH_ROOT)/dev),)
    LOCAL_ICERA_FW_PATH_DEV=$(LOCAL_ICERA_FW_PATH_ROOT)/dev
else
    LOCAL_ICERA_FW_PATH_DEV=$(LOCAL_ICERA_FW_PATH_ROOT)
endif

# Is there a prod folder?
ifneq ($(wildcard $(LOCAL_ICERA_FW_PATH_ROOT)/prod),)
    LOCAL_ICERA_FW_PATH_PROD=$(LOCAL_ICERA_FW_PATH_ROOT)/prod
else
    LOCAL_ICERA_FW_PATH_PROD=$(LOCAL_ICERA_FW_PATH_ROOT)
endif

# If "product" build - point to prod folder if exists
LOCAL_ICERA_FW_PATH=$(LOCAL_ICERA_FW_PATH_DEV)
ifneq (,$(filter $(TARGET_BUILD_TYPE)-$(TARGET_BUILD_VARIANT),release-user release-userdebug))
    LOCAL_ICERA_FW_PATH=$(LOCAL_ICERA_FW_PATH_PROD)
endif

# Embed both eu and na firmware
PRODUCT_COPY_FILES += \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_FW_PATH)/secondary_boot.wrapped:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729-voice-3g/secondary_boot.wrapped) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_FW_PATH)/loader.wrapped:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729-voice-3g/loader.wrapped) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_FW_PATH)/modem.wrapped:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729-voice-3g/modem.wrapped) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_FW_PATH)/audioConfig.bin:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729-voice-3g/audioConfig.bin) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_FW_PATH)/productConfigXml_icera_e1729_tn8l_voice_3g.bin:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729-voice-3g/productConfig.bin)

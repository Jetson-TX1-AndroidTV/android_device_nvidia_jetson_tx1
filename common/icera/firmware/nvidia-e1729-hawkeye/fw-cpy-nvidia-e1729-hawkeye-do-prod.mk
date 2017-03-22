# Copyright (c) 2014 NVIDIA Corporation.  All rights reserved.

SYSTEM_ICERA_FW_PATH=$(TARGET_COPY_OUT_VENDOR)/firmware/icera
LOCAL_ICERA_FW_PATH=vendor/nvidia/tegra/icera/firmware/binaries/binaries_nvidia-e1729-hawkeye/prod
LOCAL_ICERA_NALA_FW_PATH=vendor/nvidia/tegra/icera/firmware/binaries/binaries_nvidia-e1729-hawkeye-nala/prod

ifeq ($(wildcard $(LOCAL_ICERA_NALA_FW_PATH)),)
    # No specific nala folder
    LOCAL_ICERA_NALA_FW_PATH=$(LOCAL_ICERA_FW_PATH)
endif

# Embed both eu and na firmware
PRODUCT_COPY_FILES += \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_FW_PATH)/secondary_boot.wrapped:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729/secondary_boot.wrapped) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_FW_PATH)/loader.wrapped:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729/loader.wrapped) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_FW_PATH)/modem.wrapped:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729/modem.wrapped) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_FW_PATH)/audioConfig.bin:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729/audioConfig.bin) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_FW_PATH)/productConfigXml_icera_e1729_hawkeye.bin:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729/productConfig.bin) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_NALA_FW_PATH)/secondary_boot.wrapped:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729-nala/secondary_boot.wrapped) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_NALA_FW_PATH)/loader.wrapped:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729-nala/loader.wrapped) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_NALA_FW_PATH)/modem.wrapped:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729-nala/modem.wrapped) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_NALA_FW_PATH)/audioConfig.bin:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729-nala/audioConfig.bin) \
        $(call add-to-product-copy-files-if-exists, $(LOCAL_ICERA_NALA_FW_PATH)/productConfigXml_icera_e1729_hawkeye_nala.bin:$(SYSTEM_ICERA_FW_PATH)/nvidia-e1729-nala/productConfig.bin)

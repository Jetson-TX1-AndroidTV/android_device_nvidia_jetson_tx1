# Copyright (c) 2014, NVIDIA CORPORATION.  All rights reserved.

# Common stuff for tablet products

PRODUCT_LOCALES += en_US

# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_CHARACTERISTICS := tablet

# Set default USB interface
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp \
    persist.sys.media.avsync=true

# Set default JIT compilation filter for faster app installation
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    pm.dexopt.install=interpret-only

DEVICE_PACKAGE_OVERLAYS := $(LOCAL_PATH)/../../common/overlay-common/$(PLATFORM_VERSION_LETTER_CODE)

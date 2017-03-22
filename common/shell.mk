# Copyright (c) 2014 NVIDIA Corporation.  All rights reserved.

TARGET_PROVIDES_INIT_RC := true

PRODUCT_COPY_FILES += \
    system/core/rootdir/init.rc:root/init.base.rc \
    $(LOCAL_PATH)/init.shell.rc:root/init.rc

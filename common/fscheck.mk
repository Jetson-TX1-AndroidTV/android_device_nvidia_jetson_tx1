# Copyright (c) 2016 NVIDIA Corporation.  All rights reserved.

TARGET_USERIMAGES_USE_F2FS := true

PRODUCT_COPY_FILES += \
    device/nvidia/common/nv_setupfs.sh:system/bin/nv_setupfs.sh \
    device/nvidia/common/init.fscheck.rc:root/init.fscheck.rc

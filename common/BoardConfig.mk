# Copyright (c) 2013-2016, NVIDIA CORPORATION.  All rights reserved.
# Build definitions common to all NVIDIA boards.

# If during build configuration setup i.e. during choosecombo or lunch or
# using $TOP/buildspec.mk TARGET_PRODUCT is set to one of Nvidia boards then
# REFERENCE_DEVICE is the same as TARGET_DEVICE. For boards derived from
# NVIDIA boards, REFERENCE_DEVICE should be set to the NVIDIA
# reference device name in BoardConfig.mk or in the shell environment.

REFERENCE_DEVICE ?= $(TARGET_DEVICE)
TARGET_USES_PYTHON_IN_VENDOR := true

TARGET_RELEASETOOLS_EXTENSIONS := device/nvidia/common

ifeq ($(SECURE_OS_BUILD),tlk)
    # enable secure HDCP for secure OS build
	BOARD_VENDOR_HDCP_ENABLED ?= true
	BOARD_ENABLE_SECURE_HDCP ?= 1
	BOARD_VENDOR_HDCP_PATH ?= vendor/nvidia/tegra/tests-partner/hdcp
endif

# SELinux policy for Android M or N
BOARD_SEPOLICY_DIRS += device/nvidia/common/sepolicy_$(PLATFORM_VERSION_LETTER_CODE)

NV_BUILD_GL_SUPPORT ?= 0
# EGL_OPENGL_API support requires Android modifications only present in
# the NV and Generic Soc branches of Android.
ifeq ($(NV_ANDROID_FRAMEWORK_ENHANCEMENTS),TRUE)
NV_BUILD_GL_SUPPORT := 1
endif
ifeq ($(NV_GENERIC_SOC),1)
NV_BUILD_GL_SUPPORT := 1
endif
ifeq ($(NV_EXPOSE_GLES_ONLY),true)
NV_BUILD_GL_SUPPORT := 0
endif

# If full OpenGL is built into the OS, then export the
# feature tag to Android, so that apps can filter on the
# feature in the Play Store
ifeq ($(NV_BUILD_GL_SUPPORT),1)
PRODUCT_COPY_FILES += \
    device/nvidia/common/com.nvidia.feature.opengl4.xml:system/etc/permissions/com.nvidia.feature.opengl4.xml
endif


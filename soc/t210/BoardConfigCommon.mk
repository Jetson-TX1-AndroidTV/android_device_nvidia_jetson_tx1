TARGET_BOARD_PLATFORM := tegra
TARGET_TEGRA_VERSION := t210
TARGET_TEGRA_FAMILY := t21x

# 64-bit CPU options
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 :=
TARGET_CPU_SMP := true
TARGET_CPU_VARIANT := generic
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv7-a-neon
TARGET_2ND_CPU_VARIANT := cortex-a15
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi

# malloc on 16-byte boundary
BOARD_MALLOC_ALIGNMENT := 16

TARGET_USES_64_BIT_BINDER := true

BOARD_BUILD_BOOTLOADER := false

TARGET_USE_DTB := true

BOARD_USES_GENERIC_AUDIO := false
BOARD_USES_ALSA_AUDIO := true

ifeq ($(NV_ANDROID_FRAMEWORK_ENHANCEMENTS_AUDIO), TRUE)
USE_CUSTOM_AUDIO_POLICY := 1
else
USE_CUSTOM_AUDIO_POLICY := 0
endif

TARGET_USERIMAGES_USE_EXT4 := true
BOARD_FLASH_BLOCK_SIZE := 4096

USE_E2FSPROGS := true
USE_OPENGL_RENDERER := true

# Allow this variable to be overridden to n for non-secure OS build
SECURE_OS_BUILD ?= y
ifeq ($(SECURE_OS_BUILD),y)
    SECURE_OS_BUILD := tlk
endif

# Uncomment below line to use Nvidia's GPU-accelerated RS driver by default
# OVERRIDE_RS_DRIVER := libnvRSDriver.so

include device/nvidia/common/BoardConfig.mk

# BOARD_WIDEVINE_OEMCRYPTO_LEVEL
# The security level of the content protection provided by the Widevine DRM plugin depends
# on the security capabilities of the underlying hardware platform.
# There are Level 1/2/3. To run HD contents, should be Widevine level 1 security.
BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 1

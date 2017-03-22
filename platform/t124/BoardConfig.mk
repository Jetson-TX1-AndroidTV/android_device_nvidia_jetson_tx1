include device/nvidia/soc/t210/BoardConfigCommon.mk

TARGET_SYSTEM_PROP    += device/nvidia/soc/t210/system.prop
TARGET_SYSTEM_PROP    += device/nvidia/platform/t210/system.prop

ifeq ($(TARGET_PRODUCT),$(filter $(TARGET_PRODUCT),foster_e_hdd foster_e_ironfist_hdd foster_e_ronan_hdd))
TARGET_RECOVERY_FSTAB := device/nvidia/platform/t210/fstab_m.foster_e_hdd
BOARD_USERDATAIMAGE_PARTITION_SIZE  := 493631595008
else ifneq ($(filter falcon% hawkeye%, $(TARGET_PRODUCT)), )
TARGET_RECOVERY_FSTAB := device/nvidia/platform/t210/fstab_m.falcon
BOARD_USERDATAIMAGE_PARTITION_SIZE  := 10099646976
else ifeq ($(TARGET_PRODUCT),$(filter $(TARGET_PRODUCT),foster_e foster_e_ironfist foster_e_ronan))
TARGET_RECOVERY_FSTAB := device/nvidia/platform/t210/fstab_m.foster_e
BOARD_USERDATAIMAGE_PARTITION_SIZE  := 10099646976
else ifeq ($(TARGET_PRODUCT),$(filter $(TARGET_PRODUCT),darcy darcy_ironfist))
TARGET_RECOVERY_FSTAB := device/nvidia/platform/t210/fstab_m.darcy
BOARD_USERDATAIMAGE_PARTITION_SIZE  := 10099646976
BOARD_XUSB_FW_IN_ROOT := true
else
BOARD_USERDATAIMAGE_PARTITION_SIZE  := 10099646976
TARGET_RECOVERY_FSTAB := device/nvidia/platform/t210/fstab.t210ref
endif


ifeq ($(TARGET_PRODUCT),$(filter $(TARGET_PRODUCT),t210ref t210ref_gms t210ref_int))
ENABLE_CPUSETS := true
endif

BOARD_REMOVES_RESTRICTED_CODEC := false

#remove restricted codec for Jetson
ifneq ($(findstring t210ref, $(TARGET_PRODUCT)),)
BOARD_REMOVES_RESTRICTED_CODEC := true
endif

# TARGET_KERNEL_DT_NAME := tegra210-grenada
TARGET_KERNEL_DT_NAME := tegra210-

BOARD_SUPPORT_NVOICE := true

BOARD_SUPPORT_NVAUDIOFX :=true

BOARD_SYSTEMIMAGE_PARTITION_SIZE := 2147483648
ifneq ($(filter t210ref%,$(TARGET_PRODUCT)),)
BOARD_VENDORIMAGE_PARTITION_SIZE := 1610612736
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 2684354560
else
BOARD_VENDORIMAGE_PARTITION_SIZE := 805306368
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 2147483648
endif
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
TARGET_COPY_OUT_VENDOR := vendor

# OTA
TARGET_RECOVERY_UPDATER_LIBS += libnvrecoveryupdater
TARGET_RECOVERY_UPDATER_EXTRA_LIBS += libfs_mgr
TARGET_RECOVERY_UI_LIB := librecovery_ui_default libfscheck
TARGET_RELEASETOOLS_EXTENSIONS := device/nvidia/common
TARGET_NVPAYLOAD_UPDATE_LIB := libnvblpayload_updater

BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR ?= device/nvidia/platform/t210/bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_BCM := true

# powerhal
BOARD_USES_POWERHAL := true

# Wifi related defines
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
WPA_SUPPLICANT_VERSION      := VER_0_8_X
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
BOARD_WLAN_DEVICE           := bcmdhd
BOARD_HOSTAPD_DRIVER        := NL80211
BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_bcmdhd
WIFI_DRIVER_FW_PATH_STA     := "/data/misc/wifi/firmware/fw_bcmdhd.bin"
WIFI_DRIVER_FW_PATH_AP      := "/data/misc/wifi/firmware/fw_bcmdhd_apsta.bin"
WIFI_DRIVER_FW_PATH_P2P     := "/data/misc/wifi/firmware/fw_bcmdhd_p2p.bin"
WIFI_DRIVER_FW_PATH_PARAM   := "/sys/module/bcmdhd/parameters/firmware_path"
WIFI_DRIVER_OP_MODE_PARAM   := "/sys/module/bcmdhd/parameters/op_mode"
WIFI_DRIVER_MODULE_ARG      := "iface_name=wlan0"
WIFI_DRIVER_MODULE_NAME     := "bcmdhd"
# Default HDMI mirror mode
# Crop (default) picks closest mode, crops to screen resolution
# Scale picks closest mode, scales to screen resolution (aspect preserved)
# Center picks a mode greater than or equal to the panel size and centers;
#     if no suitable mode is available, reverts to scale
BOARD_HDMI_MIRROR_MODE := Scale

# NVDPS can be enabled when display is set to continuous mode.
BOARD_HAS_NVDPS := true

BOARD_SUPPORT_SIMULATION := true
SIM_KERNEL_DT_NAME := tegra210-grenada

# Double buffered display surfaces reduce memory usage, but will decrease performance.
# The default is to triple buffer the display surfaces.
# BOARD_DISABLE_TRIPLE_BUFFERED_DISPLAY_SURFACES := true

# Use CMU-style config with Nvcms
NVCMS_CMU_USE_CONFIG := false

# Dalvik option
DALVIK_ENABLE_DYNAMIC_GC := true

# Using the NCT partition
TARGET_USE_NCT := true

#Display static images for charging
BOARD_CHARGER_STATIC_IMAGE := true

#Use tegra health HAL library
BOARD_HAL_STATIC_LIBRARIES := libhealthd.tegra

# Enable Paragon filesystem solution.
BOARD_SUPPORT_PARAGON_FUSE_UFSD := true


# Enable verified boot for Hawkeye, Falcon, Jetson-CV, Darcy, and Loki
ifneq ($(filter hawkeye% falcon% t210ref% darcy% loki_e%, $(TARGET_PRODUCT)),)
ifneq ($(TARGET_BUILD_VARIANT),eng)
BOARD_SUPPORT_VERIFIED_BOOT := true
endif
endif

# Enable rollback protection for all devices except for Jetson
ifeq ($(filter t210ref, $(TARGET_PRODUCT)),)
BOARD_SUPPORT_ROLLBACK_PROTECTION := true
endif

# Icera modem definitions
-include device/nvidia/common/icera/BoardConfigIcera.mk

# Raydium touch definitions
include device/nvidia/drivers/touchscreen/raydium/BoardConfigRaydium.mk

# Sharp touch definitions
include device/nvidia/drivers/touchscreen/sharp/BoardConfigSharp.mk

ifneq ($(filter falcon% hawkeye%, $(TARGET_PRODUCT)), )
# Nvidia touch definitions
include device/nvidia/drivers/touchscreen/nvtouch/BoardConfigNvtouch.mk
endif

# Enable kernel compression for darcy/foster_e, falcon, hawkeye and t210ref
# supported compress method: gzip,lz4
ifneq ($(filter falcon% hawkeye% t210ref%, $(TARGET_PRODUCT)), )
BOARD_SUPPORT_KERNEL_COMPRESS := lz4
else ifneq ($(filter darcy% foster_e%, $(TARGET_PRODUCT)), )
BOARD_SUPPORT_KERNEL_COMPRESS := gzip
endif

# sepolicy
BOARD_SEPOLICY_DIRS += device/nvidia/platform/t210/sepolicy/

# seccomp policy
BOARD_SECCOMP_POLICY = device/nvidia/platform/t210/seccomp/

# Per-application sizes for shader cache
MAX_EGL_CACHE_SIZE := 128450560
MAX_EGL_CACHE_ENTRY_SIZE := 262144

# GPU/EMC boosting for hwcomposer yuv packing
HWC_YUV_PACKING_CPU_FREQ_MIN := -1
HWC_YUV_PACKING_CPU_FREQ_MAX := -1
HWC_YUV_PACKING_CPU_FREQ_PRIORITY := 15
HWC_YUV_PACKING_GPU_FREQ_MIN := 691200
HWC_YUV_PACKING_GPU_FREQ_MAX := 998400
HWC_YUV_PACKING_GPU_FREQ_PRIORITY := 15
HWC_YUV_PACKING_EMC_FREQ_MIN := 106560

# GPU/EMC floor for glcomposer composition
HWC_GLCOMPOSER_CPU_FREQ_MIN := -1
HWC_GLCOMPOSER_CPU_FREQ_MAX := -1
HWC_GLCOMPOSER_CPU_FREQ_PRIORITY := 15
HWC_GLCOMPOSER_GPU_FREQ_MIN := 614400
HWC_GLCOMPOSER_GPU_FREQ_MAX := 998400
HWC_GLCOMPOSER_GPU_FREQ_PRIORITY := 15
HWC_GLCOMPOSER_EMC_FREQ_MIN := 4080

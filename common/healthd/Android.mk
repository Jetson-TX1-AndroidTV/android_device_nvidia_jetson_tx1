LOCAL_PATH:= $(call my-dir)

include $(NVIDIA_DEFAULTS)
LOCAL_NVIDIA_NO_COVERAGE := true

LOCAL_SRC_FILES:= healthd_board_tegra.cpp
LOCAL_C_INCLUDES:= system/core/healthd/include/healthd

LOCAL_MODULE:= libhealthd.tegra
include $(NVIDIA_STATIC_LIBRARY)


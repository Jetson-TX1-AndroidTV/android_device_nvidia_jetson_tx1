LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += bootable/recovery
LOCAL_C_INCLUDES += system/core/fs_mgr/include
LOCAL_C_INCLUDES += system/extras/ext4_utils
LOCAL_C_INCLUDES += system/core/libfscheck
LOCAL_C_INCLUDES += external/libselinux/include
LOCAL_SRC_FILES := recovery_ui.cpp

# should match TARGET_RECOVERY_UI_LIB set in BoardConfig.mk
LOCAL_MODULE := librecovery_ui_default

ifeq ($(PLATFORM_IS_AFTER_LOLLIPOP),1)
LOCAL_CFLAGS += -DPLATFORM_IS_AFTER_LOLLIPOP=1
endif

ifeq ($(BOARD_SUPPORTS_BILLBOARD), true)
LOCAL_CFLAGS += -DBOARD_SUPPORTS_BILLBOARD
LOCAL_SRC_FILES += recovery_ui_billboard.cpp
LOCAL_STATIC_LIBRARIES := libpng
endif

include $(BUILD_STATIC_LIBRARY)

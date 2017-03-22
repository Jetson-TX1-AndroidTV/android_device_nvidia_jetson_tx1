# NVIDIA Tegra7 "ERS" development system
#
# Copyright (c) 2014-2016, NVIDIA Corporation.  All rights reserved.

## This is the file that is common for all ERS skus.

PRODUCT_PACKAGES += \
        nfc.tegra \
        SoundRecorder

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.barometer.xml:system/etc/permissions/android.hardware.sensor.barometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.ambient_temperature.xml:system/etc/permissions/android.hardware.sensor.ambient_temperature.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    $(LOCAL_PATH)/nvcamera.conf:system/etc/nvcamera.conf

# copy camera module definition files that will be used by config_camera.sh script at boot time
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/jetson_cv_cameras.def:system/etc/jetson_cv_cameras.def \
	$(LOCAL_PATH)/jetson_e_cameras.def:system/etc/jetson_e_cameras.def

# copy all possible feature xml files to /system/etc/camera_repo/ folder so that it can be used by config_camera.sh script at boot time.
PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.camera.full.xml:system/etc/camera_repo/android.hardware.camera.full.xml \
	frameworks/native/data/etc/android.hardware.camera.external.xml:system/etc/camera_repo/android.hardware.camera.external.xml \
	frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/camera_repo/android.hardware.camera.front.xml \
	frameworks/native/data/etc/android.hardware.camera.raw.xml:system/etc/camera_repo/android.hardware.camera.raw.xml \
	frameworks/native/data/etc/android.hardware.camera.manual_sensor.xml:system/etc/camera_repo/android.hardware.camera.manual_sensor.xml \
	frameworks/native/data/etc/android.hardware.camera.xml:system/etc/camera_repo/android.hardware.camera.xml \
	frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/camera_repo/android.hardware.camera.flash-autofocus.xml \
	frameworks/native/data/etc/android.hardware.camera.autofocus.xml:system/etc/camera_repo/android.hardware.camera.autofocus.xml \
	frameworks/native/data/etc/android.hardware.camera.manual_postprocessing.xml:system/etc/camera_repo/android.hardware.camera.manual_postprocessing.xml

ifeq ($(NV_ANDROID_FRAMEWORK_ENHANCEMENTS),TRUE)
PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/media_profiles.xml:system/etc/camera_repo/media_profiles.xml
else
PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/media_profiles_noenhance.xml:system/etc/camera_repo/media_profiles.xml
endif

ifeq ($(NV_ANDROID_FRAMEWORK_ENHANCEMENTS_AUDIO),TRUE)
PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/audio_policy.conf:system/etc/audio_policy.conf
else
PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/audio_policy_noenhance.conf:system/etc/audio_policy.conf
endif

# jetson's media profiles
PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/jetson_e_media_profiles.xml:system/etc/camera_repo/jetson_e_media_profiles.xml \
  $(LOCAL_PATH)/jetson_cv_media_profiles.xml:system/etc/camera_repo/jetson_cv_media_profiles.xml \
  $(LOCAL_PATH)/jetson_cv_e3323_media_profiles.xml:system/etc/camera_repo/jetson_cv_e3323_media_profiles.xml \
  $(LOCAL_PATH)/jetson_cv_e3333_media_profiles.xml:system/etc/camera_repo/jetson_cv_e3333_media_profiles.xml \
  $(LOCAL_PATH)/jetson_cv_imx274_media_profiles.xml:system/etc/camera_repo/jetson_cv_imx274_media_profiles.xml \
  $(LOCAL_PATH)/jetson_cv_nocam_media_profiles.xml:system/etc/camera_repo/jetson_cv_nocam_media_profiles.xml \

PRODUCT_COPY_FILES += \
    device/nvidia/platform/t210/nvaudio_conf.xml:system/etc/nvaudio_conf.xml

#symlinks
PRODUCT_PACKAGES += \
    camera.autofocus.symlink \
    camera.external.symlink \
    camera.flash-autofocus.symlink \
    camera.front.symlink \
    camera.full.symlink \
    camera.manual_sensor.symlink \
    camera.manual_postprocessing.symlink \
    camera.raw.symlink \
    camera.symlink \
    camera.media.symlink

# add vendor public preloaded libs
PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/vendor.libraries.android.txt:vendor/etc/public.libraries.txt

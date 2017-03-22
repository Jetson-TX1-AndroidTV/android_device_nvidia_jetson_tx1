#!/system/bin/sh

# Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

# config_cameras.sh -- Configure camera features and media_profiles.xml
#                      depending on camera modules

# For more information, please see:
#   https://wiki.nvidia.com/wmpwiki/index.php/Camera/Android/Configuration

########################################################################
# Wrapper functions
########################################################################

set_camera_feature() {
    local feature="$1"
    local value="$2"

    # remove an existing feature file
    rm -f /data/camera_config/etc/permissions/android.hardware.${feature}.xml

    # create a symbolic link to /data/camera_config/etc/permissions folder
    if [[ "$value" == "true" ]]; then
        ln -s /system/etc/camera_repo/android.hardware.${feature}.xml /data/camera_config/etc/permissions/android.hardware.${feature}.xml
    fi
}

# set camera feature only if the second parameter is 'true' or 'false'
set_feature() {
    if [[ "$2" == "true" || "$2" == "false" ]]; then
        set_camera_feature $1 $2
    fi
}

enable_autofocus () {
    set_feature "camera.autofocus" $1
}

enable_external () {
    set_feature "camera.external" $1
}

enable_flash_autofocus () {
    set_feature "camera.flash-autofocus" $1
}

enable_front () {
    set_feature "camera.front" $1
}

enable_full () {
    set_feature "camera.full" $1
}

enable_manual_postprocessing () {
    set_feature "camera.manual_postprocessing" $1
}

enable_manual_sensor () {
    set_feature "camera.manual_sensor" $1
}

enable_raw () {
    set_feature "camera.raw" $1
}

enable_camera () {
    set_feature "camera" $1
}

use_media_profiles() {
    local fileList="$1"
    local wordCount="$(echo \"$fileList\" | wc -w)"
    local firstFile="${fileList%% *}"
    local remainingFiles="${fileList#* }"

    # create symbolic link for the first file and rename it to media_profiles.xml
    # (FYI, another method to specify a path to media_profiles.xml is using 'setprop' at boot time:
    #  'setprop media.settings.xml [path to media_profile.xml]')
    ln -f -s $firstFile /data/camera_config/etc/media_profiles.xml

    if [[ "$wordCount" -gt 1 ]]; then
        # create symbolic links for remaining files
        for file in $remainingFiles; do
            ln -f -s $file /data/camera_config/etc/$(basename $file)
        done
    fi
}

########################################################################
# Module Definitions
# (the following information is used if it is not overrided by
#  the module definition file.)
########################################################################

AVAILABLE_CAM_BOARD_IDS=""

on_module_default() {
    enable_autofocus              true
    enable_external               false
    enable_flash_autofocus        false
    enable_front                  true
    enable_full                   true
    enable_manual_postprocessing  true
    enable_manual_sensor          true
    enable_raw                    true
    enable_camera                 true
    use_media_profiles            "/system/etc/camera_repo/media_profiles.xml"
}


########################################################################
# Main function
########################################################################


if [[ "$1" == "load" ]]; then

  moduleDefFile="$2"

  if [[ ! -f "$moduleDefFile" ]]; then
      echo "[Error] module definition file $moduleDefFile doesn't exist!"
      exit 1
  fi

  # override AVAILABLE_CAM_BOARD_IDS and module definitions
  . ${moduleDefFile}

  matched="false"
  AVAILABLE_CAM_BOARD_IDS="${AVAILABLE_CAM_BOARD_IDS}"

  for id in ${AVAILABLE_CAM_BOARD_IDS}; do
      ret=`ls /proc/device-tree/chosen/plugin-manager/ids/${id}-* | wc -l`
      if [ "$ret" != "0" ]; then
          eval "on_module_$id"
          matched="true"
          break
      fi
  done

  # run default function if there is no matching ID
  if [[ "$matched" == "false" ]]; then
      on_module_default
  fi
  exit 0
elif [[ "$1" == "set" ]]; then
  # call method
  $2 $3
else
  echo "config_cameras.sh -- Configure camera features and media_profiles.xml"
  echo "                     depending on camera modules."
  echo "                     (used only by init.*.rc at boot time)"
  echo ""
  echo "Usage>"
  echo "  config_cameras.sh load [path to the camera module definition file]"
  echo "    : load and configure camera module"
fi

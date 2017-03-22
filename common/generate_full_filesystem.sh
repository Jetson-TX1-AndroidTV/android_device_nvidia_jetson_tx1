#!/bin/bash
# Copyright (c) 2011-2015 NVIDIA Corporation.  All rights reserved.
#
# NVIDIA Corporation and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA Corporation is strictly prohibited.
#
# Usage: generate_full_filesystem.sh $(TARGET_DEVICE)
#        generate_full_filesystem.sh --help
#
# Description: Generate a complete Android root file system, both in a ramdisk
#              and in a folder that can be mounted through NFS.
#
#              TARGET_DEVICE is an Android build variable.
#-------------------------------------------------------------------------------

# Help target
if [ "$1" == "--help" ]; then
    this_script=`basename $0`
    echo " "
    echo "Generate a complete Android root file system, both in a ramdisk"
    echo "and in a folder that can be mounted through NFS."
    echo " "
    echo "Usage:"
    echo " "
    echo "  $this_script $(TARGET_DEVICE)"
    echo "  $this_script --help"
    echo " "
    echo "where:"
    echo " "
    echo " " TARGET_DEVICE is an Android build variable, you can find it
    echo " " out by running "get_build_var TARGET_DEVICE"
    echo " "
    echo "  --help: produces this description"
    echo " "
    exit 0
fi

nfs_out=$OUT/full_filesystem
ramdisk_out=$OUT/full_filesystem.img

android_host_bin=$ANDROID_HOST_OUT/bin

# Sanity checks
if [ "$OUT" == "" ]; then
    echo "ERROR: Environment variable OUT is not set. You must initialize an Android build shell."
    exit 2
fi

if [ "$ANDROID_HOST_OUT" == "" ]; then
    echo "ERROR: Environment variable ANDROID_HOST_OUT is not set. You must initialize an Android build shell."
    exit 2
fi

if [ "$1" == "" -o "$1" == "t210_int" -o "$1" == "t210" ]; then
    echo "Assuming target board is t210ref"
    target_device="t210"
else
    target_device=$1
fi

if [ ! -d "$android_host_bin" ]; then
    echo "ERROR: Host tools directory $android_host_bin does not exist."
    exit 2
fi

# Output directory preparation
echo "Generating NFS-friendly root file system under $nfs_out."
rm -rf $nfs_out
mkdir -p $nfs_out

# File copy
cp -ra $OUT/root/* $nfs_out
cp -ra $OUT/system $nfs_out
cp -ra $OUT/data $nfs_out

device_dir=$TOP/device/nvidia/platform/$target_device
shell_init_rc=$device_dir/init_no_root_device.rc
root_init_rc=$OUT/root/init.rc

if [ -f "$shell_init_rc" ]; then
    echo cp $shell_init_rc $nfs_out/init.rc
    cp $shell_init_rc $nfs_out/init.rc
    echo cp $root_init_rc $nfs_out/init.base.rc
    cp $root_init_rc $nfs_out/init.base.rc
fi

# Ensure that group and world do not have write permissions for certain files in
# $nfs_out. If the following files have group/world write permissions, the init
# executable complains about "skipping insecure files" and simulator does not
# boot up.
chmod 644 $nfs_out/*.rc
chmod 644 $nfs_out/default.prop
chmod 644 $nfs_out/system/build.prop

# remove iptables, netd and dnsmasq to speed up ASIM
rm $nfs_out/system/bin/iptables
rm $nfs_out/system/bin/ip6tables
rm $nfs_out/system/bin/dnsmasq
rm $nfs_out/system/bin/netd

# Package into a ramdisk
echo "Packaging ramdisk as $ramdisk_out."
# Start WAR bug 200073250
# Drop the .apk from the ramdisk image to keep it small enough to
# boot on ASIM. The NFS folder still contains everything.
ramdisk_tmp=$OUT/ramdisk_tmp
rm -rf $ramdisk_tmp
mkdir -p $ramdisk_tmp
cp -ra $nfs_out/* $ramdisk_tmp
find $ramdisk_tmp -name "*.apk" -print | xargs rm
$android_host_bin/mkbootfs $ramdisk_tmp | gzip -9 > $ramdisk_out
#$android_host_bin/mkbootfs $nfs_out | gzip -9 > $ramdisk_out
# End WAR bug 200073250

echo "Done"
exit 0


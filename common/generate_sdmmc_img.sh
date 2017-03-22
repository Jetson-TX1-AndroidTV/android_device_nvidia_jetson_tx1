#!/bin/bash
#
# Usage: generate_sdmmc_img.sh
#
#        This does not require any argument, but it may prompt you to
#        type root password.
#
# Prerequisites:
#        user has to setup the build environment first, then make an
#        Android build. This will set up "OUT" environmental variable
#        and create system.img file, which is required by this script.
#
# Description:
#        Generate an image simulating a EMMC card or SDMMC card,
#        which can be passed to cosim and libasim.so for ASIM
#        simulation.
#
#        here is how to verify the card content:
#
# 1. sudo losetup -o 1048576 --sizelimit 510656512 /dev/loop1 $OUT/sdmmc.img
# 2. sudo mount /dev/loop1 <your_mount_point>
#
#        then you can enter <your_mount_point> to see the contents.

dd bs=512 count=4M if=/dev/zero of=$OUT/sdmmc.img
$ANDROID_HOST_OUT/bin/simg2img $OUT/system.img $OUT/system.img.raw
sudo losetup /dev/loop0 $OUT/sdmmc.img
sudo parted /dev/loop0 mklabel gpt
sudo parted /dev/loop0 mkpart APP 1M 512M
sudo parted /dev/loop0 mkpart UDA 513M 768M
sudo parted /dev/loop0 mkpart CAC 769M 1024M
sudo parted /dev/loop0 mkpart MSC 1025M 1268M
sudo parted /dev/loop0 print
sudo losetup -d /dev/loop0
sudo losetup -o 1048576 --sizelimit 510656512 /dev/loop1 $OUT/sdmmc.img
sudo dd if=$OUT/system.img.raw of=/dev/loop1
sudo losetup -d /dev/loop1

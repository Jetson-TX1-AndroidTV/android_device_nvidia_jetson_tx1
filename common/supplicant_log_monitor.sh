#!/system/bin/sh

# Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

touch /data/misc/wifi/supplicant.log
touch /data/misc/wifi/supplicant_old.log
chmod 0660 /data/misc/wifi/supplicant.log
chmod 0660 /data/misc/wifi/supplicant_old.log
chown system:wifi /data/misc/wifi/supplicant.log
chown system:wifi /data/misc/wifi/supplicant_old.log

FILESIZE_SUPPLICANT=0
FILENAME_SUPPLICANT=/data/misc/wifi/supplicant.log

while
do
FILESIZE_SUPPLICANT=$(stat -c%s "$FILENAME_SUPPLICANT")
if [ "$FILESIZE_SUPPLICANT" -gt 100000000 ]; then
    /system/bin/log -t "IDS filesize check" -p i "Taking backup of supplicant"
    cp /data/misc/wifi/supplicant.log /data/misc/wifi/supplicant_old.log
    truncate -s 0 /data/misc/wifi/supplicant.log
fi
sleep 60
done

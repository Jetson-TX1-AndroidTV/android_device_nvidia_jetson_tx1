#!/system/bin/sh

# Copyright (c) 2012-2016, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

if [ $(getprop ro.boot.commchip_id) -gt 0 ]; then
	/system/bin/log -t "wifiloader" -p i "setting user configured value of WiFi chipset"
	setprop wifi.commchip_id $(getprop ro.boot.commchip_id)
	exit
fi

# -----------------------------------------------------------------------------
# SDIO Wifi Chip Detection
# -----------------------------------------------------------------------------

# vendor id defines
BRCM=0x02d0
TI=0x0097
MRVL=0x02df
automotive_device=n
hardware= ($(getprop ro.hardware))

# get wifi country code from factory partition
wifi_country_code=
if [ -f /mnt/factory/wifi/country.txt ]; then
	wifi_country_code=`cat /mnt/factory/wifi/country.txt`
fi
/system/bin/log -t "wifiloader" -p i "Factory partition wifi country code: $wifi_country_code"

#Sometime WiFi card enumeration gets delayed in kernel. Retry 3 times for WiFi card enumeration from kernel.
COUNT=0;
while [ $COUNT -le 3 ]; do
	#find hardware used and assigned corresponding mmc interface used for wifi chip
	sdmmc=$(getprop ro.wifi.sdmmc)
	if [ -z $sdmmc ]; then
		/system/bin/log -t "wifiloader" -p i "ro.wifi.sdmmc property not set, reading from sdmmc0"
		mmc=$(ls /sys/bus/platform/devices/sdhci-tegra.0/mmc_host)
	else
		/system/bin/log -t "wifiloader" -p i "ro.wifi.sdmmc property set, reading from sdmmc$sdmmc"
		mmc=$(ls /sys/bus/platform/devices/sdhci-tegra.$sdmmc/mmc_host)
	fi
	/system/bin/log -t "wifiloader" -p i "Using $mmc for WiFi"

	vendor=$(cat /sys/bus/sdio/devices/$mmc:0001:1/vendor)
	device=$(cat /sys/bus/sdio/devices/$mmc:0001:1/device)
	if [ -z $vendor ]; then
		/system/bin/log -t "wifiloader" -p e "WiFi card is not available! try $COUNT"
		sleep 2
		COUNT=$(($COUNT+1))
	else
		/system/bin/log -t "wifiloader" -p i "WiFi VendorID: $vendor, DeviceID: $device"
		break
	fi
done
if [ -z $vendor ]; then
	/system/bin/log -t "wifiloader" -p e "WiFi auto card detection fail"
fi

COUNT=0;
if [ -e /system/lib/modules/bluedroid_pm.ko ]; then
	/system/bin/log -t "wifiloader" -p i "Bluedroid_pm driver compiled as module"
	while [ $COUNT -le 5 ]; do
		if [ '1' -eq `lsmod | grep -c bluedroid_pm` ]; then
			/system/bin/log -t "wifiloader" -p i "Bluedroid_pm driver loaded at $COUNT iteration"
			break
		fi
		sleep 1
		COUNT=$(($COUNT+1))
		if [ $COUNT -eq 5 ]; then
			/system/bin/log -t "wifiloader" -p e "Failed to detect Bluedroid_pm driver load"
		fi
	done
fi
vendor_device="$vendor"_"$device"
vendor_device_country="$vendor"_"$device"_"$wifi_country_code"
#Read vendor and product idea enumerated during kernel boot
if [ -z "$(getprop persist.sys.commchip_vendor)" ]; then
	/system/bin/log -t "wifiloader" -p i "persist.sys.commchip_vendor not defined. Reading enumerated data"
	setprop persist.sys.commchip_vendor $vendor
	setprop persist.sys.commchip_device $device
	setprop persist.sys.commchip_country "$wifi_country_code"
elif [ $vendor_device_country = $(getprop persist.sys.commchip_vendor)"_"$(getprop persist.sys.commchip_device)"_"$(getprop persist.sys.commchip_country) ]; then
	/system/bin/log -t "wifiloader" -p i "persist.sys.commchip_vendor defined by user. Using user-defined config"
else
	/system/bin/log -t "wifiloader" -p i "Comm chip replaced by user. reset symlinks if needed"
	if [ "$vendor" = "$BRCM" ]; then
		/system/bin/rm /data/misc/wifi/firmware/fw_bcmdhd.bin
		/system/bin/rm /data/misc/wifi/firmware/fw_bcmdhd_apsta.bin
		/system/bin/rm /data/misc/wifi/firmware/nvram.txt
		if [ $device = "43341" ]; then
			/system/bin/rm /data/misc/wifi/firmware/fw_bcmdhd_a0.bin
			/system/bin/rm /data/misc/wifi/firmware/fw_bcmdhd_apsta_a0.bin
			/system/bin/rm /data/misc/wifi/firmware/nvram_43341_rev3.txt
			/system/bin/rm /data/misc/wifi/firmware/nvram_43341_rev4.txt
		fi

	fi
	setprop persist.sys.commchip_vendor $vendor
	setprop persist.sys.commchip_device $device
	setprop persist.sys.commchip_country "$wifi_country_code"
#Enable below two when there is switch to other vendor solutions because
#other vendor requires their conf files.
#	/system/bin/rm /data/misc/wifi/wpa_supplicant.conf
#	/system/bin/rm /data/misc/wifi/p2p_supplicant.conf
fi

if [ "$(getprop ro.auto.device)" = "true" ]; then
	/system/bin/log -t "wifiloader" -p i "automotive device is detected"
	automotive_device=y
fi

#Find device and set configurations
#broadcomm comm chip
if [ "$vendor" = "$BRCM" ]; then
	if [ $device = "0x4334" ]; then
		/system/bin/log -t "wifiloader" -p i  "BCM4334 chip identified"
		chip="4334"
	elif [ $device = "0xa94d" ]; then
		/system/bin/log -t "wifiloader" -p i  "BCM43341 chip identified"
		chip="43341"
	elif [ $device = "0x4339" ] && [ $automotive_device = y ]; then
		/system/bin/log -t "wifiloader" -p i  "BCM89335 chip identified"
		chip="4339"
	elif [ $device = "0x4324" ] && [ $automotive_device = y ]
	then
		/system/bin/log -t "wifiloader" -p i  "BCM43241-B4 chip identified"
		chip="43241-b4"
	elif [ $device = "0x4324" ]; then
		/system/bin/log -t "wifiloader" -p i  "BCM43241 chip identified"
		chip="43241"
	elif [ $device = "0x4335" ]; then
                /system/bin/log -t "wifiloader" -p i  "BCM4335 chip identified"
                chip="4335"
	elif [ $device = "0x4350" ]; then
		/system/bin/log -t "wifiloader" -p i  "BCM4350 chip identified"
		chip="4350"
	elif [ $device = "0x4354" ]; then
		/system/bin/log -t "wifiloader" -p i  "BCM4354 chip identified"
		chip="4354"
	fi

	/system/bin/log -t "wifiloader" -p i  "chip=$chip hardware=$hardware wifi_country_code=$wifi_country_code"

	if [ -e /data/misc/wifi/firmware ];then
		rm -rf /data/misc/wifi/firmware
	fi

#marvel comms chip
elif [ $vendor_device = "$MRVL""_0x9129" ]; then
	/system/bin/log -t "wifiloader" -p i  "MRVL8797 chip identified"
	if [ ! -f /data/misc/wifi/wpa_supplicant.conf ]; then
		cp /system/etc/firmware/marvel_wpa.conf /data/misc/wifi/wpa_supplicant.conf
		chmod 0660 /data/misc/wifi/wpa_supplicant.conf
		chown system:wifi /data/misc/wifi/wpa_supplicant.conf
	fi
	if [ ! -f /data/misc/wifi/p2p_supplicant.conf ]; then
		cp /system/etc/firmware/marvel_p2p.conf /data/misc/wifi/p2p_supplicant.conf
		chmod 0660 /data/misc/wifi/p2p_supplicant.conf
		chown system:wifi /data/misc/wifi/p2p_supplicant.conf
	fi
	if [ -e /system/lib/modules/sd8797mlan.ko ]; then
		insmod /system/lib/modules/sd8797mlan.ko
		insmod /system/lib/modules/sd8797.ko drv_mode=5 cfg80211_wext=12 fw_name=sd8797_uapsta.bin max_vir_bss=1 sta_name=wlan wfd_name=p2p
		insmod /system/lib/modules/mbt8797.ko fw_name=sd8797_uapsta.bin
		chown bluetooth net_bt_stack /dev/mbtchar0
	else
		/system/bin/log -t "wifiloader" -p i "KO not found, compiled part of kernel"
	fi
elif [ $vendor_device = "$MRVL""_0x912d" ]; then
	/system/bin/log -t "wifiloader" -p i  "MRVL8897 chip identified"
	if [ ! -f /data/misc/wifi/wpa_supplicant.conf ]; then
		cp /system/etc/firmware/marvel_wpa.conf /data/misc/wifi/wpa_supplicant.conf
		chmod 0660 /data/misc/wifi/wpa_supplicant.conf
		chown system:wifi /data/misc/wifi/wpa_supplicant.conf
	fi
	if [ ! -f /data/misc/wifi/p2p_supplicant.conf ]; then
		cp /system/etc/firmware/marvel_p2p.conf /data/misc/wifi/p2p_supplicant.conf
		chmod 0660 /data/misc/wifi/p2p_supplicant.conf
		chown system:wifi /data/misc/wifi/p2p_supplicant.conf
	fi
	if [ -e /system/lib/modules/sd8897mlan.ko ]; then
		insmod /system/lib/modules/sd8897mlan.ko
		insmod /system/lib/modules/sd8897.ko drv_mode=5 cfg80211_wext=12 fw_name=sd8897_uapsta.bin max_vir_bss=1 sta_name=wlan wfd_name=p2p p2p_enh=1
		insmod /system/lib/modules/mbt8897.ko fw_name=sd8897_uapsta.bin
		chown bluetooth net_bt_stack /dev/mbtchar0
	else
		/system/bin/log -t "wifiloader" -p i "KO not found, compiled part of kernel"
	fi
fi

# -----------------------------------------------------------------------------
# PCI Wifi Chip Detection
# -----------------------------------------------------------------------------

# vendor id defines
BRCM=0x14e4

for path in /sys/bus/pci/devices/*; do
	vendor=$(cat $path/vendor)
	if [ "$vendor" = "$BRCM" ]; then
		device=$(cat $path/device)
		break;
	fi
done

if [ "$vendor" = "$BRCM" ]; then
	if [ $device = "0x4354" -o $device = "0x43ec" ]; then
		/system/bin/log -t "wifiloader" -p i  "BCM4356 chip identified"
		chip="4356"
	elif [ $device = "0x4355" -o $device = "0x43ef" ]; then
		/system/bin/log -t "wifiloader" -p i  "BCM89359-B0/B1 chip identified"
		chip="4359"
		chip_rev0="_b0"
		chip_rev1="_b1"
	fi

	if [ ! -e /data/misc/wifi/firmware/fw_bcmdhd.bin$chip_rev0 ]; then
		/system/bin/log -t "wifiloader" -p i  "create fw_bcmdhd.bin$chip_rev0 soft link"
		/system/bin/ln -s /system/vendor/firmware/bcm$chip$chip_rev0/fw_bcmdhd.bin /data/misc/wifi/firmware/fw_bcmdhd.bin$chip_rev0
	fi
	if [ ! -e /data/misc/wifi/firmware/fw_bcmdhd.bin$chip_rev1 ]; then
		/system/bin/log -t "wifiloader" -p i  "create fw_bcmdhd.bin$chip_rev1 soft link"
		/system/bin/ln -s /system/vendor/firmware/bcm$chip$chip_rev1/fw_bcmdhd.bin /data/misc/wifi/firmware/fw_bcmdhd.bin$chip_rev1
	fi

	if [ ! -e /data/misc/wifi/firmware/fw_bcmdhd_apsta.bin$chip_rev0 ]; then
		/system/bin/log -t "wifiloader" -p i  "create fw_bcmdhd_apsta.bin$chip_rev0 soft link"
		/system/bin/ln -s /system/vendor/firmware/bcm$chip$chip_rev0/fw_bcmdhd.bin /data/misc/wifi/firmware/fw_bcmdhd_apsta.bin$chip_rev0
	fi
	if [ ! -e /data/misc/wifi/firmware/fw_bcmdhd_apsta.bin$chip_rev1 ]; then
		/system/bin/log -t "wifiloader" -p i  "create fw_bcmdhd_apsta.bin$chip_rev1 soft link"
		/system/bin/ln -s /system/vendor/firmware/bcm$chip$chip_rev1/fw_bcmdhd.bin /data/misc/wifi/firmware/fw_bcmdhd_apsta.bin$chip_rev1
	fi

	if [ ! -e /data/misc/wifi/firmware/nvram.txt$chip_rev0 ]; then
		if [[ $hardware == *"p2382"* ]]; then
			/system/bin/log -t "wifiloader" -p i  "create murata$chip_rev0 nvram soft link"
			/system/bin/ln -s /system/etc/nvram_$chip$chip_rev0.txt /data/misc/wifi/firmware/nvram.txt$chip_rev0
		else
			/system/bin/log -t "wifiloader" -p i  "create nvram.txt$chip_rev0 soft link"
			/system/bin/ln -s /system/etc/nvram_$chip$chip_rev0.txt /data/misc/wifi/firmware/nvram.txt$chip_rev0
		fi
	fi
	if [ ! -e /data/misc/wifi/firmware/nvram.txt$chip_rev1 ]; then
		if [[ $hardware == *"p2382"* ]]; then
			/system/bin/log -t "wifiloader" -p i  "create murata$chip_rev1 nvram soft link"
			/system/bin/ln -s /system/etc/nvram_murata_$chip$chip_rev1.txt /data/misc/wifi/firmware/nvram.txt$chip_rev1
		else
			/system/bin/log -t "wifiloader" -p i  "create nvram.txt$chip_rev1 soft link"
			/system/bin/ln -s /system/etc/nvram_$chip$chip_rev1.txt /data/misc/wifi/firmware/nvram.txt$chip_rev1
		fi
	fi
	/system/bin/log -t "wifiloader" -p i  "setting up symlink for firmware_path"
	/system/bin/ln -s /sys/module/bcmdhd/parameters/firmware_path /data/misc/wifi/firmware/firmware_path
fi

#bluetooth_log
touch /data/misc/bluedroid_log/data.txt
chown bluetooth:system /data/misc/bluedroid_log/data.txt
touch /data/misc/bluedroid_log/log.txt
chown bluetooth:system /data/misc/bluedroid_log/log.txt

datafile="/data/misc/wifi/wifi_scan_config.conf"
etcfile="/etc/wifi/wifi_scan_config.conf"

if ! /system/bin/cmp -s $datafile $etcfile ; then
	/system/bin/log -t "wifiloader" -p i  "Linking Scan config file"
	/system/bin/rm $datafile
	/system/bin/ln -s $etcfile $datafile
fi

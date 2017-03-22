#!/system/bin/sh

# choose first udc
UDC_NAME=$(set `ls /sys/class/udc` && echo $1)
setprop sys.usb.udc $UDC_NAME

DELAY=$(getprop sys.usb.delay)
CONFIG=$(getprop sys.usb.config)

sleep ${DELAY:-10}

setprop ffs.ready 1
setprop sys.usb.config ${CONFIG:-mtp,adb}
setprop sys.usb.delay 2

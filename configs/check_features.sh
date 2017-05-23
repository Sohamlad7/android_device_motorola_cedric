#!/sbin/sh

sku=`getprop ro.boot.hardware.sku`

if [ "$sku" = "XT1687" ]; then
    # XT1687 doesn't have NFC chip
    rm /system/etc/permissions/android.hardware.nfc.xml
    rm /system/etc/permissions/android.hardware.nfc.hce.xml
    rm /system/etc/permissions/com.android.nfc_extras.xml
else
    # Only XT1687 variant got a compass
    rm /system/etc/permissions/android.hardware.sensor.compass.xml
fi

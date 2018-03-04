#!/vendor/bin/sh

PATH=/sbin:/vendor/bin
export PATH

scriptname=${0##*/}

# We take this from cpuinfo because hex "letters" are lowercase there - (lolwhat)
cinfo=`getprop ro.boot.hwrev`
hw=${cinfo#???}

# Now "cook" the value so it can be matched against devtree names
m2=${hw%?}
minor2=${hw#$m2}
m1=${m2%?}
minor1=${m2#$m1}
if [ "$minor2" == "0" ]; then
	minor2=""
	if [ "$minor1" == "0" ]; then
		minor1=""
	fi
fi

rev="p${hw%??}$minor1$minor2"
rev2=`echo $rev | tr '[:upper:]' '[:lower:]'`

setprop ro.boot.hardware.revision $rev2
unset hw cinfo m1 m2 minor1 minor2

# Let kernel know our image version/variant/crm_version
if [ -f /sys/devices/soc0/select_image ]; then
    image_version="10:"
    image_version+=`getprop ro.build.id`
    image_version+=":"
    image_version+=`getprop ro.build.version.incremental`
    image_variant=`getprop ro.product.name`
    image_variant+="-"
    image_variant+=`getprop ro.build.type`
    oem_version=`getprop ro.build.version.codename`
    echo 10 > /sys/devices/soc0/select_image
    echo $image_version > /sys/devices/soc0/image_version
    echo $image_variant > /sys/devices/soc0/image_variant
    echo $oem_version > /sys/devices/soc0/image_crm_version
fi



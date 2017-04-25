#!/system/bin/sh

PATH=/sbin:/system/sbin:/system/bin:/system/xbin
export PATH

while getopts dpfr op;
do
	case $op in
		d)  dbg_on=1;;
		p)  populate_only=1;;
		f)  dead_touch=1;;
		r)  reset_touch=1;;
	esac
done
shift $(($OPTIND-1))

scriptname=${0##*/}
hw_mp=/proc/hw
config_mp=/proc/config
reboot_utag=$hw_mp/.reboot
touch_status_prop=hw.touch.status
hw_cfg_file=hw_config.xml
vhw_file=/system/etc/vhw.xml
bp_file=/system/build.prop
oem_file=/oem/oem.prop
load_error=3
need_to_reload=2
reload_in_progress=1
reload_done=0
ver_utag=".version"
version_fs="unknown"
xml_version="unknown"
device_params=""
xml_file=""
set -A prop_names
set -A prop_overrides
prop_names=(ro.product.device ro.product.name)

debug()
{
	[ $dbg_on ] && echo "Debug: $*"
}

notice()
{
	echo "$*"
	echo "$scriptname: $*" > /dev/kmsg
}

reload_utags()
{
	local mp=$1
	local value
	echo "1" > $mp/reload
	value=$(cat $mp/reload)
	while [ "$value" == "$reload_in_progress" ]; do
		notice "waiting for loading to complete"
		sleep 1;
		value=$(cat $mp/reload)
		notice "'$mp' current status [$value]"
	done
}

procfs_wait_for_device()
{
	local __result=$1
	local status
	local mpi
	local IFS=' '
	while [ ! -f $hw_mp/reload ] || [ ! -f $config_mp/reload ]; do
		notice "waiting for devices"
		sleep 1;
        done
	for mpi in $hw_mp; do
		status=$(cat $mpi/reload)
		notice "mount point '$mpi' status [$status]"
		if [ "$status" == "$need_to_reload" ]; then
			notice "force $mpi reloading"
			reload_utags $mpi
		fi
	done
	for mpi in $hw_mp; do
		status=$(cat $mpi/reload)
		notice "$mpi reload is [$status]"
		while [ "$status" != "$reload_done" ]; do
			notice "waiting for loading to complete"
			sleep 1;
			status=$(cat $mpi/reload)
		done
	done
	eval $__result=$status
}



set_ro_hw_properties()
{
	local utag_path
	local utag_name
	local prop_prefix
	local utag_value
	local verify
	for hwtag in $(find $hw_mp -name '.system'); do
		debug "path $hwtag has '.system' in its name"
		prop_prefix=$(cat $hwtag/ascii)
		verify=${prefix%.}
		# esure property ends with '.'
		if [ "$prop_prefix" == "$verify" ]; then
			prop_prefix="$prop_prefix."
			debug "added '.' at the end of [$prop_prefix]"

                fi
		utag_path=${hwtag%/*}
		utag_name=${utag_path##*/}
		utag_value=$(cat $utag_path/ascii)
		setprop $prop_prefix$utag_name "$utag_value"
		notice "ro.hw.$utag_name='$utag_value'"
	done
}

# Main starts here
IFS=$'\n'

notice "initializing procfs"
procfs_wait_for_device readiness

set_ro_hw_properties

return 0


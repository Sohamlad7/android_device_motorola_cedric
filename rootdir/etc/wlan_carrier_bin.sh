#!/vendor/bin/sh
carrier=`getprop ro.boot.carrier`

case "$carrier" in
    "retbr" | "timbr" | "tefbr" | "oibr" | "amxbr" | "niibr")
        echo -n wlan/prima/WCNSS_qcom_wlan_nv_Brazil.bin > /sys/module/wcnsscore/parameters/nv_file
    ;;
    "perar" | "retar" | "tefar" | "amxar")
        echo -n wlan/prima/WCNSS_qcom_wlan_nv_Argentina.bin > /sys/module/wcnsscore/parameters/nv_file
    ;;
    "retin" | "amzin")
        echo -n wlan/prima/WCNSS_qcom_wlan_nv_India.bin > /sys/module/wcnsscore/parameters/nv_file
    ;;
    * )
        echo -n wlan/prima/WCNSS_qcom_wlan_nv.bin > /sys/module/wcnsscore/parameters/nv_file
    ;;
esac

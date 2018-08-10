#
# Copyright (C) 2017 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

[AID_VENDOR_RFS]
value:2951

[AID_VENDOR_RFS_SHARED]
value:2952

[system/vendor/bin/wcnss_filter]
mode: 0755
user: AID_BLUETOOTH
group: AID_BLUETOOTH
caps: BLOCK_SUSPEND

[system/vendor/bin/cnss-daemon]
mode: 0755
user: AID_BLUETOOTH
group: AID_BLUETOOTH
caps: NET_BIND_SERVICE

[system/vendor/bin/imsdatadaemon]
user: AID_SYSTEM
group: AID_SYSTEM
mode: 0755
caps: NET_BIND_SERVICE

[system/vendor/bin/ims_rtp_daemon]
user: AID_SYSTEM
group: AID_RADIO
mode: 0755
caps: NET_BIND_SERVICE

[system/vendor/bin/pm-service]
user: AID_SYSTEM
group: AID_SYSTEM
mode: 0755
caps: NET_BIND_SERVICE

[system/vendor/bin/cnd]
mode: 0755
user: AID_SYSTEM
group: AID_SYSTEM
caps: NET_BIND_SERVICE BLOCK_SUSPEND NET_ADMIN

[system/vendor/bin/slim_daemon]
mode: 0755
user:  AID_GPS
group: AID_GPS
caps: NET_BIND_SERVICE

[system/vendor/bin/xtwifi-client]
mode: 0755
user:  AID_GPS
group: AID_GPS
caps: NET_BIND_SERVICE BLOCK_SUSPEND

[system/vendor/bin/mm-qcamera-daemon]
mode: 0700
user: AID_CAMERA
group: AID_SHELL
caps: SYS_NICE

[firmware/]
mode: 0771
user: AID_SYSTEM
group: AID_SYSTEM
caps: 0

[persist/]
mode: 0771
user: AID_SYSTEM
group: AID_SYSTEM
caps: 0

[AID_VENDOR_MOT_ACCY]
value: 5000

[AID_VENDOR_MOT_PWRIC]
value: 5001

[AID_VENDOR_MOT_USB]
value: 5002

[AID_VENDOR_MOT_DRM]
value: 5003

[AID_VENDOR_MOT_TCMD]
value: 5004

[AID_VENDOR_MOT_SEC_RTC]
value: 5005

[AID_VENDOR_MOT_TOMBSTONE]
value: 5006

[AID_VENDOR_MOT_TPAPI]
value: 5007

[AID_VENDOR_MOT_SECCLKD]
value: 5008

[AID_VENDOR_MOT_WHISPER]
value: 5009

[AID_VENDOR_MOT_CAIF]
value: 5010

[AID_VENDOR_MOT_DLNA]
value: 5011

[AID_VENDOR_MOT_ATVC]
value: 5012

[AID_VENDOR_MOT_DBVC]
value: 5014

[AID_VENDOR_FINGERP]
value: 5015

[AID_VENDOR_MOT_ESDFS]
value: 5016

[AID_VENDOR_MOT_ESDFS]
value: 5016

[AID_VENDOR_MOT_ESDFS]
value: 5016

[AID_VENDOR_POWER]
value: 5017

[AID_VENDOR_ITSON]
value: 5018

[AID_VENDOR_MOT_DTV]
value: 5019

[AID_VENDOR_MOT_MOD]
value: 5020

[AID_VENDOR_MOT_SHARED]
value: 5323

[AID_VENDOR_MOT_COMMON]
value: 5341

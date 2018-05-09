#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

# ADSP
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/tinyalsa/include
LOCAL_SHARED_LIBRARIES := libtinyalsa
LOCAL_SRC_FILES := mixer.c
LOCAL_MODULE := libshimalsa
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := camera_hal.cpp
LOCAL_SHARED_LIBRARIES := libEGL
LOCAL_MODULE := libWSL
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := MediaCodec.cpp
LOCAL_SHARED_LIBRARIES := libstagefright libmedia
LOCAL_MODULE := libWshimfright
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := libqsap_shim.c
LOCAL_SHARED_LIBRARIES := libqsap_sdk liblog
LOCAL_C_INCLUDES := $(TOP)/system/qcom/softap/sdk
LOCAL_MODULE := libqsapshim
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)

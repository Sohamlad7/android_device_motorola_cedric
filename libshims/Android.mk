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
LOCAL_SRC_FILES := mixer.c
LOCAL_MODULE := libshim_adsp
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := camera_hal.cpp
LOCAL_MODULE := libshim_camera_hal
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libgui
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := MediaCodec.cpp

LOCAL_SHARED_LIBRARIES := libstagefright libmedia

LOCAL_MODULE := libshims_camera
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)


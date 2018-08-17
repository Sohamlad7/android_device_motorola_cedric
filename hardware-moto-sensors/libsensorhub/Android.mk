# Copyright (C) 2009-2015 Motorola Mobility, Inc.
# Copyright (C) 2008 The Android Open Source Project
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

include $(CLEAR_VARS)

LOCAL_MODULE := libsensorhub
LOCAL_MODULE_TAGS := optional

ifeq ($(MOT_SENSOR_HUB_HW_TYPE_L4), true)
    LOCAL_CFLAGS += -DMOTOSH
else ifeq ($(MOT_SENSOR_HUB_HW_TYPE_L0), true)
    LOCAL_CFLAGS += -DSTML0XX
endif

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
    LOCAL_CFLAGS += -DDEBUG
endif

LOCAL_SRC_FILES :=              \
    SensorHub.cpp Endian.cpp

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
# Need the UAPI output directory to be populated with motosh.h/stml0xx.h
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := liblog libcutils libutils libc
LOCAL_CFLAGS += -Wall -Wextra
LOCAL_CFLAGS += -Wno-gnu-designator -Wno-writable-strings
LOCAL_CXXFLAGS += -Weffc++ -std=c++14

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)


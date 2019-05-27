# Copyright (C) 2017 Motorola Mobility, Inc.
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

LOCAL_MODULE := sensors.tof.vl53l0
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -Wno-gnu-designator -Wno-writable-strings

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
    LOCAL_CFLAGS += -DDEBUG
endif

LOCAL_CFLAGS += -DLOG_TAG=\"ToFSensors\"

LOCAL_SRC_FILES := \
    TofModule.cpp \
    RearProxSensor.cpp \
    SensorsPollContext.cpp \
    SensorBase.cpp \
    ../InputEventReader.cpp

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/.. \
    system/core/base/include \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_C_INCLUDES += external/zlib

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

# Needs to be added after KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += kernel/include

LOCAL_HEADER_LIBRARIES := libutils_headers libhardware_headers
LOCAL_SHARED_LIBRARIES += liblog libcutils libutils libc libbase
LOCAL_CFLAGS += -Wall -Wextra
LOCAL_CFLAGS += -Wno-gnu-designator -Wno-writable-strings
LOCAL_CXXFLAGS += -Weffc++ -std=c++14

#LOCAL_PRELINK_MODULE := false

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)


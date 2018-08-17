# Copyright (C) 2009-2014 Motorola Mobility, Inc.
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

LOCAL_TOP_DIR := $(call my-dir)
LOCAL_PATH := $(LOCAL_TOP_DIR)
LOCAL_CFLAGS += -Wno-gnu-designator -Wno-writable-strings

# If other Android.mk files are included explicitly, this must be called before
# including those.
include $(call all-subdir-makefiles)

# Restore LOCAL_PATH. Other makefiles probably modified it.
LOCAL_PATH := $(LOCAL_TOP_DIR)

###########################################
# Motorola SensorHub section only         #
# Sensors are connected to motorola       #
# internal sensorhub like STM             #
###########################################
ifeq ($(BOARD_USES_MOT_SENSOR_HUB), true)

    ifneq ($(TARGET_SIMULATOR),true)

        UTILS_PATH := utils

        ###################################
        # Select sensorhub processor type #
        ###################################
        ifeq ($(MOT_SENSOR_HUB_HW_TYPE_L4), true)
            SH_MODULE := motosh
            SH_PATH := motosh_hal
            ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
                # Expose IR raw data for non-user builds
                SH_CFLAGS += -D_ENABLE_RAW_IR_DATA
            endif
        else ifeq ($(MOT_SENSOR_HUB_HW_TYPE_L0), true)
            SH_MODULE := stml0xx
            SH_PATH := stml0xx_hal
        endif

        ##########################
        # Select sensor hardware #
        ##########################
        ifeq ($(MOT_SENSOR_HUB_HW_BMI160), true)
            ifeq ($(MOT_SENSOR_HUB_HW_TYPE_L4), true)
                SH_CFLAGS += -D_USES_BMI160_ACCGYR
            else ifeq ($(MOT_SENSOR_HUB_HW_TYPE_L0), true)
                SH_CFLAGS += -D_ENABLE_BMI160
                SH_CFLAGS += -D_ENABLE_GYROSCOPE
            endif
        endif

        ifeq ($(MOT_AP_SENSOR_HW_REARPROX_2), true)
            SH_CFLAGS += -D_ENABLE_REARPROX_2
        endif

        ifeq ($(MOT_AP_SENSOR_HW_REARPROX), true)
            SH_CFLAGS += -D_ENABLE_REARPROX
        endif

        ifeq ($(MOT_AP_SENSOR_HW_CAPSENSE), true)
	    SH_CFLAGS += -D_ENABLE_CAPSENSE
        endif

        ifeq ($(MOT_SENSOR_HUB_HW_IR), true)
            SH_CFLAGS += -D_ENABLE_IR
        endif

        ifeq ($(MOT_SENSOR_HUB_HW_AK09912), true)
            SH_CFLAGS += -D_ENABLE_MAGNETOMETER
        endif

        ##########################
        # Select sensorhub algos #
        ##########################
        ifeq ($(MOT_SENSOR_HUB_FEATURE_CHOPCHOP), true)
            SH_CFLAGS += -D_ENABLE_CHOPCHOP
        endif

        ifeq ($(MOT_SENSOR_HUB_FEATURE_LIFT), true)
            SH_CFLAGS += -D_ENABLE_LIFT
        endif

        ifeq ($(MOT_SENSOR_HUB_FEATURE_PEDO), true)
            SH_CFLAGS += -D_ENABLE_PEDO
        endif

        ifeq ($(MOT_SENSOR_HUB_FEATURE_LA), true)
            SH_CFLAGS += -D_ENABLE_LA
        endif

        ifeq ($(MOT_SENSOR_HUB_FEATURE_GR), true)
            SH_CFLAGS += -D_ENABLE_GR
        endif
        ifeq ($(MOT_SENSOR_HUB_FEATURE_CAMFSYNC), true)
            SH_CFLAGS += -D_CAMFSYNC
        endif
        ifeq ($(MOT_SENSOR_HUB_FEATURE_ULTRASOUND), true)
            SH_CFLAGS += -D_ENABLE_ULTRASOUND
        endif

        ######################
        # Sensors HAL module #
        ######################
        include $(CLEAR_VARS)

        LOCAL_CFLAGS := -DLOG_TAG=\"MotoSensors\"
        LOCAL_CFLAGS += $(SH_CFLAGS)
        LOCAL_CXX_FLAGS += -std=c++14

        LOCAL_SRC_FILES :=              \
            $(SH_PATH)/SensorBase.cpp   \
            $(SH_PATH)/SensorHal.cpp    \
            $(SH_PATH)/HubSensors.cpp   \
            $(SH_PATH)/SensorList.cpp

        ifeq ($(MOT_SENSOR_HUB_HW_TYPE_L0), true)
            # Sensor HAL file for M0 hub (low-tier) products (athene, etc...)
            LOCAL_SRC_FILES += \
                $(SH_PATH)/Quaternion.cpp \
                $(SH_PATH)/GyroIntegration.cpp \
                $(SH_PATH)/GameRotationVector.cpp \
                $(SH_PATH)/LinearAccelGravity.cpp
            ifeq ($(MOT_SENSOR_HUB_HW_AK09912), true)
                LOCAL_SRC_FILES += \
                    $(SH_PATH)/GeoMagRotationVector.cpp \
                    $(SH_PATH)/RotationVector.cpp
            endif

        endif

        ifeq ($(MOT_SENSOR_HUB_HW_TYPE_L4), true)
            LOCAL_REQUIRED_MODULES += sensors.iio
        endif

        ifeq ($(MOT_AP_SENSOR_HW_REARPROX), true)
            LOCAL_SRC_FILES += \
                $(SH_PATH)/RearProxSensor.cpp \
                InputEventReader.cpp
        endif

        ifeq ($(MOT_AP_SENSOR_HW_CAPSENSE), true)
            LOCAL_SRC_FILES += \
                $(SH_PATH)/CapSense.cpp
            ifneq ($(MOT_AP_SENSOR_HW_REARPROX), true)
                LOCAL_SRC_FILES += InputEventReader.cpp
            endif
        endif

        # This file must be last, for some mysterious reason
        LOCAL_SRC_FILES += \
            $(SH_PATH)/SensorsPollContext.cpp

        LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(SH_PATH)
        LOCAL_C_INCLUDES += external/zlib

        LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                            system/core/base/include

        # Needs to be added after KERNEL_OBJ/usr/include
        ifeq ($(MOT_SENSOR_HUB_HW_TYPE_L4), true)
            LOCAL_C_INCLUDES += kernel/include
        endif

        # Need the UAPI output directory to be populated with motosh.h/stml0xx.h
        LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE_RELATIVE_PATH := hw
        LOCAL_MODULE_TAGS := optional
        LOCAL_PROPRIETARY_MODULE := true
        LOCAL_SHARED_LIBRARIES += liblog libcutils libz libdl libutils
        LOCAL_CLANG := true
        LOCAL_MODULE := sensors.$(TARGET_BOARD_PLATFORM)
        LOCAL_CFLAGS += -Wno-gnu-designator -Wno-writable-strings

        include $(BUILD_SHARED_LIBRARY)

    endif # !TARGET_SIMULATOR

    #########################
    # Sensor Hub HAL module #
    #########################
    include $(CLEAR_VARS)

    LOCAL_PRELINK_MODULE := false
    LOCAL_MODULE_RELATIVE_PATH := hw
    LOCAL_SRC_FILES := $(SH_PATH)/sensorhub.c
    LOCAL_SRC_FILES += $(UTILS_PATH)/sensor_time.cpp

    LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
    # Need the UAPI output directory to be populated with motosh.h/stml0xx.h
    LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

    LOCAL_SHARED_LIBRARIES := libcutils libc libutils liblog
    LOCAL_PROPRIETARY_MODULE := true
    LOCAL_MODULE := sensorhub.$(TARGET_BOARD_PLATFORM)
    LOCAL_MODULE_TAGS := optional
    LOCAL_CFLAGS += -Wno-gnu-designator -Wno-writable-strings

    include $(BUILD_SHARED_LIBRARY)

    ###########################
    # Sensor Hub Flash loader #
    ###########################
    include $(CLEAR_VARS)

    LOCAL_REQUIRED_MODULES := sensorhub.$(TARGET_BOARD_PLATFORM)
    LOCAL_REQUIRED_MODULES += sensors.$(TARGET_BOARD_PLATFORM)

    LOCAL_MODULE_TAGS := optional
    LOCAL_CFLAGS := -DLOG_TAG=\"MOTOSH\" -DMODULE_$(SH_MODULE)
    LOCAL_MODULE := motosh
    LOCAL_PROPRIETARY_MODULE := true
    #LOCAL_CFLAGS+= -D_DEBUG
    LOCAL_CFLAGS += -Wall -Wextra
    LOCAL_CFLAGS += -Wno-gnu-designator -Wno-writable-strings
    # Added by top level make files: -std=gnu++11
    LOCAL_CXXFLAGS += -Weffc++
    LOCAL_SHARED_LIBRARIES := libcutils libc libsensorhub liblog

    LOCAL_SRC_FILES := \
        motosh_bin/motosh.cpp \
        motosh_bin/CRC32.c
    LOCAL_REQUIRED_MODULES += sensorhub-blacklist.txt


    LOCAL_C_INCLUDES := \
        $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
        hardware/moto/sensors/libsensorhub

    # Need the UAPI output directory to be populated with motosh.h/stml0xx.h
    LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

    include $(BUILD_EXECUTABLE)

    #########################
    # AKM executable        #
    #########################
    ifeq ($(MOT_SENSOR_HUB_HW_AK09912), true)
        include $(CLEAR_VARS)

        AKM_PATH := ak09912_akmd_6D_32b
        SMARTCOMPASS_LIB := libSmartCompass

        LOCAL_MODULE_TAGS := optional

        LOCAL_MODULE  := akmd09912

        LOCAL_C_INCLUDES := \
            $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
            $(LOCAL_PATH)/$(AKM_PATH) \
            $(LOCAL_PATH)/$(AKM_PATH)/$(SMARTCOMPASS_LIB) \
            $(LOCAL_PATH)/$(SH_PATH)

        LOCAL_SRC_FILES := \
            $(AKM_PATH)/AKMD_Driver.c \
            $(AKM_PATH)/DispMessage.c \
            $(AKM_PATH)/FileIO.c \
            $(AKM_PATH)/Measure.c \
            $(AKM_PATH)/main.c \
            $(AKM_PATH)/misc.c \
            $(AKM_PATH)/FST_AK09912.c \
            $(AKM_PATH)/Acc_aot.c

        LOCAL_CFLAGS := -DAKMD_FOR_AK09912
        LOCAL_CFLAGS += -DAKMD_AK099XX
        LOCAL_CFLAGS += -DAKMD_ACC_EXTERNAL
        LOCAL_CFLAGS += -Wall -Wextra
        #LOCAL_CFLAGS += -DENABLE_AKMDEBUG=1
        LOCAL_CFLAGS += $(SH_CFLAGS)
        LOCAL_CFLAGS += -Wno-gnu-designator -Wno-writable-strings

        LOCAL_STATIC_LIBRARIES := AK09912

        LOCAL_FORCE_STATIC_EXECUTABLE := false
        LOCAL_SHARED_LIBRARIES := libc libm libutils libcutils liblog

        include $(BUILD_EXECUTABLE)

        include $(CLEAR_VARS)
        LOCAL_MODULE        := AK09912
        LOCAL_MODULE_TAGS   := optional
        LOCAL_MODULE_CLASS  := STATIC_LIBRARIES
        LOCAL_MODULE_SUFFIX := .a
        LOCAL_SRC_FILES_arm   := $(AKM_PATH)/$(SMARTCOMPASS_LIB)/arm/libAK09912.a
        LOCAL_SRC_FILES_arm64 := $(AKM_PATH)/$(SMARTCOMPASS_LIB)/arm64/libAK09912.a
        include $(BUILD_PREBUILT)
    endif # MOT_SENSOR_HUB_HW_AK09912

    ifeq ($(BOARD_USES_CAP_SENSOR), true)
        CAP_PATH := capsensor
        include $(CLEAR_VARS)
        LOCAL_MODULE := capsense_reset
        LOCAL_SRC_FILES :=  \
            $(CAP_PATH)/capsense.cpp
        LOCAL_SHARED_LIBRARIES := libc liblog libcutils libhardware_legacy
        LOCAL_MODULE_TAGS := optional
        include $(BUILD_EXECUTABLE)
    endif

    ifeq ($(MOT_SENSOR_HUB_HW_TYPE_L0), true)
        # This creates a link from stml0xx to motosh so that code that
        # uses the old name will still work. This can be removed once
        # everything has been updated to the new name.
        OLD_SH_BIN := stml0xx
        SH_SYMLINK := $(addprefix $(TARGET_OUT)/bin/,$(OLD_SH_BIN))
        $(SH_SYMLINK): NEW_SH_BIN := $(LOCAL_MODULE)
        $(SH_SYMLINK): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
        # WARNING - the below lines must be indented with a TAB, not spaces
		@echo "Symlink: $@ -> $(NEW_SH_BIN)"
		@mkdir -p $(dir $@)
		@rm -rf $@
		$(hide) ln -sf $(NEW_SH_BIN) $@
        ALL_DEFAULT_INSTALLED_MODULES += $(SH_SYMLINK)
    endif

    # ** Firmware BlackList **********************************************************
    include $(CLEAR_VARS)
    LOCAL_MODULE        := sensorhub-blacklist.txt
    LOCAL_MODULE_TAGS   := optional
    LOCAL_MODULE_CLASS  := ETC
    LOCAL_MODULE_PATH   := $(TARGET_OUT)/etc/firmware
    LOCAL_SRC_FILES     := motosh_bin/sensorhub-blacklist.txt
    include $(BUILD_PREBUILT)
    # ********************************************************************************

else # For non sensorhub version of sensors
    ###########################################
    # No-SensorHub section only               #
    # Sensors are connected directly to AP    #
    ###########################################

endif # BOARD_USES_MOT_SENSOR_HUB


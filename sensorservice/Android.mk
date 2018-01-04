LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ../../../../frameworks/native/services/sensorservice/main_sensorservice.cpp

LOCAL_SHARED_LIBRARIES := \
    libsensorservice \
    libbinder \
    libutils

LOCAL_CFLAGS := -Wall -Werror -Wextra

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= sensorservice_32

LOCAL_32_BIT_ONLY := true
LOCAL_INIT_RC := sensorservice.rc
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)

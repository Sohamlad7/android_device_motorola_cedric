LOCAL_PATH := $(call my-dir)

# HAL module implemenation stored in
# hw/<POWERS_HARDWARE_MODULE_ID>.<ro.hardware>.so
include $(CLEAR_VARS)

LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SHARED_LIBRARIES := liblog libcutils libdl libxml2
LOCAL_SRC_FILES := power.c metadata-parser.c utils.c list.c hint-data.c powerhintparser.c
LOCAL_C_INCLUDES := external/libxml2/include \
                    external/icu/icu4c/source/common

LOCAL_SRC_FILES += power-8953.c

LOCAL_MODULE := power.msm8953
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -Wno-unused-parameter -Wno-unused-variable
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)


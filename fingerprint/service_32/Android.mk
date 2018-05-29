LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.biometrics.fingerprint@2.1-service_32
LOCAL_INIT_RC := android.hardware.biometrics.fingerprint@2.1-service_32.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_32_BIT_ONLY := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_VENDOR_MODULE := true
LOCAL_SRC_FILES := \
    ../../../../../hardware/interfaces/biometrics/fingerprint/2.1/default/BiometricsFingerprint.cpp \
    ../../../../../hardware/interfaces/biometrics/fingerprint/2.1/default/service.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libhidlbase \
    libhidltransport \
    libhardware \
    libutils \
    android.hardware.biometrics.fingerprint@2.1

include $(BUILD_EXECUTABLE)

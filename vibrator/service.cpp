/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "android.hardware.vibrator@1.0-service.cedric"

#include <android/hardware/vibrator/1.0/IVibrator.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Errors.h>
#include <utils/StrongPointer.h>

#include "Vibrator.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::vibrator::V1_0::IVibrator;
using android::hardware::vibrator::V1_0::implementation::Vibrator;
using namespace android;

#define VIBRATOR "/sys/devices/virtual/timed_output/vibrator/"

#define ENABLE      VIBRATOR "enable"

#define VTG_DEFAULT VIBRATOR "vtg_default"
#define VTG_LEVEL   VIBRATOR "vtg_level"
#define VTG_MIN     VIBRATOR "vtg_min"
#define VTG_MAX     VIBRATOR "vtg_max"

status_t registerVibratorService() {
    std::ofstream enable{ENABLE};
    if (!enable) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", ENABLE, error, strerror(error));
        return -error;
    }

    std::ifstream vtg_default{VTG_DEFAULT};
    if (!vtg_default) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", VTG_DEFAULT, error, strerror(error));
        return -error;
    }
    
    std::ofstream amplitude{VTG_LEVEL};
    if (!amplitude) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", VTG_LEVEL, error, strerror(error));
        return -error;
    }
    
    std::ifstream vtg_min{VTG_MIN};
    if (!vtg_min) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", VTG_MIN, error, strerror(error));
        return -error;
    }
    
    std::ifstream vtg_max{VTG_MAX};
    if (!vtg_max) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", VTG_MAX, error, strerror(error));
        return -error;
    }

    sp<IVibrator> vibrator = new Vibrator(std::move(enable), std::move(vtg_default), std::move(amplitude), std::move(vtg_min), std::move(vtg_max));
    (void) vibrator->registerAsService(); // suppress unused-result warning
    return OK;
}

int main() {
    configureRpcThreadpool(1, true);
    status_t status = registerVibratorService();

    if (status != OK) {
        return status;
    }

    joinRpcThreadpool();
}

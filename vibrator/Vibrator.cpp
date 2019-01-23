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

#define LOG_TAG "VibratorService"

#include <log/log.h>

#include <hardware/hardware.h>
#include <hardware/vibrator.h>

#include "Vibrator.h"

#include <cinttypes>
#include <cmath>
#include <iostream>
#include <fstream>


namespace android {
namespace hardware {
namespace vibrator {
namespace V1_0 {
namespace implementation {

#define DEFAULT_VTG 127
#define DEFAULT_MIN_VTG 32
#define DEFAULT_MAX_VTG 127

static constexpr uint32_t CLICK_TIMING_MS = 20;

static uint32_t get(std::ifstream file, uint32_t defaultValue) {
    uint32_t value = defaultValue;
    if (!file)
        return value;

    file >> value;
    return value;
}

Vibrator::Vibrator(std::ofstream&& enable, std::ifstream&& vtg_default, std::ofstream&& amplitude, std::ifstream&& vtg_min, std::ifstream&& vtg_max) :
    mEnable(std::move(enable)),
    mAmplitude(std::move(amplitude)) {
        defaultVoltage = get(std::move(vtg_default), DEFAULT_VTG);
        minVoltage = get(std::move(vtg_min), DEFAULT_MIN_VTG);
        maxVoltage = get(std::move(vtg_max), DEFAULT_MAX_VTG);
    }

// Methods from ::android::hardware::vibrator::V1_0::IVibrator follow.
Return<Status> Vibrator::on(uint32_t timeout_ms) {
    mEnable << timeout_ms << std::endl;
    if (!mEnable) {
        ALOGE("Failed to turn vibrator on (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

Return<Status> Vibrator::off()  {
    mEnable << 0 << std::endl;
    if (!mEnable) {
        ALOGE("Failed to turn vibrator off (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

Return<bool> Vibrator::supportsAmplitudeControl()  {
    return true;
}

Return<Status> Vibrator::setAmplitude(uint8_t amplitude) {
    if (amplitude == 0) {
        return Status::BAD_VALUE;
    }
    // Scale the voltage such that an amplitude of 0 is minVoltage, an amplitude of 255 is
    // maxVoltage, and there are equal steps for every value in between.
    long voltage =
            std::lround((amplitude - 1) / 255.0 * (maxVoltage - minVoltage) + minVoltage);
    ALOGE("Setting amplitude  to: %ld", voltage);
    mAmplitude << voltage << std::endl;
    if (!mAmplitude) {
        ALOGE("Failed to set amplitude (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

Return<void> Vibrator::perform(Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    if (effect == Effect::CLICK) {
        uint8_t amplitude;
        switch (strength) {
        case EffectStrength::LIGHT:
            amplitude = 128;
            break;
        case EffectStrength::MEDIUM:
            amplitude = 190;
            break;
        case EffectStrength::STRONG:
            amplitude = 255;
            break;
        default:
            _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
            return Void();
        }
        on(CLICK_TIMING_MS);
        setAmplitude(amplitude);
        _hidl_cb(Status::OK, CLICK_TIMING_MS);
    } else {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
    }
    return Void();
}

} // namespace implementation
}  // namespace V1_0
}  // namespace vibrator
}  // namespace hardware
}  // namespace android

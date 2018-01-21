/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_LASERPROX_SENSOR_H
#define ANDROID_LASERPROX_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <hardware/sensors.h>
#include <string>

#include "SensorBase.h"
#include "InputEventReader.h"
#include "SensorsLog.h"

#define ID_RP 0xFE

/*****************************************************************************/

class RearProxSensor : public SensorBase {
public:
    RearProxSensor();
    virtual ~RearProxSensor();

    virtual int setEnable(int32_t handle, int enabled) override;
    virtual int batch(int32_t handle, int flags,
            int64_t sampling_period_ns, int64_t max_report_latency_ns) override;
    virtual int readEvents(sensors_event_t* data, int count) override;
    virtual int flush(int32_t handle) override;
    virtual bool hasSensor(int handle) override {
        return (handle == ID_RP);
    }

    void processEvent(int code, int value);
    void handleSARprocess(int distance);
    int enableSAR(char enable);
    int setSARConfig(uint32_t mode);
    int findPathByAttr();
    virtual void getSensorsList(std::vector<struct sensor_t> &list) {
        list.push_back(
            { .name = "Rear Proximity sensor",
              .vendor = "Motorola",
              .version = 1,
              .handle = ID_RP,
              .type = SENSOR_TYPE_PROXIMITY,
              .maxRange = 10.0f,
              .resolution = 10.0f,
              .power = 0.35f,
              .minDelay = 0,
              .fifoReservedEventCount = 0,
              .fifoMaxEventCount = 0,
              .stringType = SENSOR_STRING_TYPE_PROXIMITY,
              .requiredPermission = "",
              .maxDelay = 10000000,
              .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
              .reserved = { 0, 0 } });
    }
private:
    uint32_t mEnabled;
    uint32_t mFlushEnabled;
    uint32_t mDetectInterval;
    uint32_t mDetectDistance;
    uint32_t mDetectWeight;
    uint32_t mDetectMode;
    uint32_t mCurrentDistance;
    bool mTimestampUpdated;
    bool mSendData;
    std::string mFilePath;
    InputEventCircularReader mInputReader;
    sensors_event_t mPendingEvents;
};

/*****************************************************************************/

#endif  // ANDROID_LASERPROX_SENSOR_H

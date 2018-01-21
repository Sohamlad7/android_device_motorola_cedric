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

#include "Sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

/*****************************************************************************/

struct input_event;

class RearProxSensor : public SensorBase {
public:
	RearProxSensor(int n);
	virtual ~RearProxSensor();

	virtual int setEnable(int32_t handle, int enabled) override;
	virtual int setDelay(int32_t handle, int64_t ns) override;
	virtual int readEvents(sensors_event_t* data, int count) override;
	virtual int flush(int32_t handle) override;

	virtual int getEnable(int32_t handle);
	void processEvent(int code, int value);
private:
	uint32_t mEnabled;
	uint32_t mPendingMask;
#ifdef _ENABLE_REARPROX_2
	uint32_t numrp;
#endif
	InputEventCircularReader mInputReader;
	sensors_event_t mPendingEvents;
	sensors_event_t mFlushEvents;
	uint64_t mDelays;
	uint32_t mFlushEnabled;
	static RearProxSensor self;
};

/*****************************************************************************/

#endif  // ANDROID_LASERPROX_SENSOR_H

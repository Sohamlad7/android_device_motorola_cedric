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

/*
 * Copyright (C) 2015-2016 Motorola Mobility LLC
 */

#ifndef SENSORS_POLL_CONTEXT_H
#define SENSORS_POLL_CONTEXT_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <map>
#include <new>

#include <linux/input.h>

#include <cutils/atomic.h>
#include <cutils/log.h>


#include "Sensors.h"
#include "SensorBase.h"


class SensorsPollContext {
public:
    sensors_poll_device_1_t device; // must be first

    SensorsPollContext();
    ~SensorsPollContext();
    static SensorsPollContext* getInstance();
    int activate(int handle, int enabled);
    int setDelay(int handle, int64_t ns);
    int pollEvents(sensors_event_t* data, int count);
    int batch(int handle, int flags, int64_t ns, int64_t timeout);
    int flush(int handle);

private:
    enum {
        sensor_hub = 0,
#ifdef _ENABLE_REARPROX
        rearprox,
#endif
#ifdef _ENABLE_REARPROX_2
        rearprox_2,
#endif
#ifdef _ENABLE_CAPSENSE
        capsense,
#endif
        numSensorDrivers,
        numFds,
    };

    static SensorsPollContext self;
    struct pollfd mPollFds[numSensorDrivers];
    SensorBase* mSensors[numSensorDrivers];

        //! \brief Map from sensor id (handle) to sensor_t entry
        std::map<int32_t, const sensor_t*> mIdToSensor;

    int handleToDriver(int handle);
};

#endif /* SENSORS_POLL_CONTEXT_H */

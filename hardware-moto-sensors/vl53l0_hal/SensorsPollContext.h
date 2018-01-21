/*
 * Copyright (C) 2017 Motorola Mobility
 *
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

#ifndef SENSORS_POLL_CONTEXT_H
#define SENSORS_POLL_CONTEXT_H

#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <new>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

#include <poll.h>
#include <pthread.h>
#include <unistd.h>

#include <linux/input.h>

#include <cutils/log.h>

#include <sys/select.h>

#include <stdint.h>
#include <endian.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <zlib.h>
#include <time.h>
#include <private/android_filesystem_config.h>
#include <iterator>
#include <android-base/macros.h>

#include "SensorBase.h"
#include "SensorsLog.h"
#include "BaseHal.h"

/**
 * This is a HAL primarily dedicated to handling ToF sensor. The bulk of
 * the work is done by the ToF Sensor driver.
 */
class SensorsPollContext : public BaseHal {
public:
    SensorsPollContext();
    virtual ~SensorsPollContext() = default;

    virtual int poll(sensors_event_t* data, int count) override;

    virtual int getSensorsList(struct sensor_t const** list) override;

private:
    DISALLOW_COPY_AND_ASSIGN(SensorsPollContext);

    std::vector<struct pollfd> pollFds;

    /// Used to map file descriptors to corresponding drivers
    std::map<int, std::shared_ptr<SensorBase>> fd2driver;
};

#endif /* SENSORS_POLL_CONTEXT_H */

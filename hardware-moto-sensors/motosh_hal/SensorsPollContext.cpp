/*
 * Copyright (C) 2009-2015 Motorola Mobility
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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#include <cutils/atomic.h>

#include <linux/input.h>

#include <sys/select.h>
#include <sys/types.h>

#include <hardware/sensors.h>

#include "SensorsPollContext.h"
#include "SensorsLog.h"

#if defined(_ENABLE_REARPROX)
#include "SensorBase.h"
#include "RearProxSensor.h"
#endif

using namespace std;

SensorsPollContext::SensorsPollContext() {
    S_LOGD("+ pid=%d tid=%d", getpid(), gettid());

    drivers.push_back(make_shared<HubSensors>());

#ifdef _ENABLE_REARPROX
    drivers.push_back(make_shared<RearProxSensor>());
#endif

    for (const auto& driver : drivers) {
        if (!driver) {
            S_LOGE("Null driver");
            continue;
        }
        int fd = driver->getFd();
        //S_LOGD("Adding fd=%d for driver=0x%08x @ %d", fd, driver.get(), pollFds.size());
        if (fd >= 0) {
            fd2driver[fd] = driver;
            pollFds.push_back({
                    .fd = fd,
                    .events = POLLIN,
                    .revents = 0
            });
        }
    }
}

int SensorsPollContext::poll(sensors_event_t* data, int count)
{
    int nbEvents = 0;
    int ret;
    int err;

    //S_LOGD("count=%d pid=%d tid=%d", count, getpid(), gettid());

    if (!data) {
        S_LOGE("poll failed, data buffer is null");
        return -EINVAL;
    }

    if (count < 1) {
        S_LOGE("poll failed, invalid event count %d", count);
        return -EINVAL;
    }

    auto eventReader = [&](shared_ptr<SensorBase> d, int fd) {
        int nb = d->readEvents(data, count, fd);
        if (nb > 0) {
            count -= nb;
            nbEvents += nb;
            data += nb;
        }
        return nb;
    };

    // See if we have any pending events before blocking on poll()
    for (const auto& d : drivers) {
        if (d->hasPendingEvents()) {
            if (eventReader(d, -1) <= 0) {
                S_LOGE("Failed to read pending events");
            }
        }
    }

    if (nbEvents) return nbEvents;

    ret = TEMP_FAILURE_RETRY(::poll(&pollFds.front(), pollFds.size(), -1));
    err = errno;

    // Success
    if (ret > 0) {
        for (const auto& p : pollFds) {
            if (p.revents & POLLIN) {
                int nb = eventReader(fd2driver[p.fd], p.fd);
                // Need to relay any errors upward.
                if (nb <= 0) {
                    S_LOGE("reading events failed fd=%d nb=%d", p.fd, nb);
                    return 0;
                }
            }
        }
    } else if (ret == 0) {
        S_LOGD("poll() timed out");
    } else {
        S_LOGE("poll() failed with %d (%s)", err, strerror(err));
    }

    return nbEvents >= 0 ? nbEvents : 0;
}

int SensorsPollContext::getSensorsList(struct sensor_t const** list) {
    S_LOGD("+");

    static vector<struct sensor_t> sensorList;
    sensorList.clear();

    for (const auto& driver : drivers) {
        driver->getSensorsList(sensorList);
    }

    *list = &sensorList[0];

    return sensorList.size();
}


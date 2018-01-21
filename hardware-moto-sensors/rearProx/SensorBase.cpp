/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Not a Contribution.
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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include <cutils/log.h>

#include <linux/input.h>

#include "NativeSensorManager.h"
#include "SensorBase.h"

/*****************************************************************************/

SensorBase::SensorBase(
        const char* dev_name,
        const char* data_name,
        const struct SensorContext* context /* = NULL */)
        : dev_name(dev_name), data_name(data_name),
        dev_fd(-1), data_fd(-1), mEnabled(0), mHasPendingMetadata(0)
{
        if (context != NULL) {
                /* Set up the sensors_meta_data_event_t event*/
                meta_data.version = META_DATA_VERSION;
                meta_data.sensor = context->sensor->handle;
                meta_data.type = SENSOR_TYPE_META_DATA;
                meta_data.reserved0 = 0;
                meta_data.timestamp = 0LL;
                meta_data.meta_data.what = META_DATA_FLUSH_COMPLETE;
                meta_data.meta_data.sensor = context->sensor->handle;
        }
        if (data_name) {
                data_fd = openInput(data_name);
        }
}

SensorBase::~SensorBase() {
    if (data_fd >= 0) {
        close(data_fd);
    }
    if (dev_fd >= 0) {
        close(dev_fd);
    }
}

int SensorBase::open_device() {
    if (dev_fd<0 && dev_name) {
        dev_fd = open(dev_name, O_RDONLY);
        ALOGE_IF(dev_fd<0, "Couldn't open %s (%s)", dev_name, strerror(errno));
    }
    return 0;
}

int SensorBase::close_device() {
    if (dev_fd >= 0) {
        close(dev_fd);
        dev_fd = -1;
    }
    return 0;
}

int SensorBase::getFd() const {
    if (!data_name) {
        return dev_fd;
    }
    return data_fd;
}

int SensorBase::setDelay(int32_t, int64_t) {
    return 0;
}

bool SensorBase::hasPendingEvents() const {
    return false;
}

int64_t SensorBase::getTimestamp() {
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_BOOTTIME, &t);
    return int64_t(t.tv_sec)*1000000000LL + t.tv_nsec;
}

int SensorBase::openInput(const char* inputName) {
    int fd = -1;
    const char *dirname = "/dev/input";
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strlcpy(devname, dirname, PATH_MAX);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
                (de->d_name[1] == '\0' ||
                        (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strlcpy(filename, de->d_name, PATH_MAX - strlen(SYSFS_CLASS));
        fd = open(devname, O_RDONLY);
        if (fd>=0) {
            char name[80];
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
                name[0] = '\0';
            }
            if (!strcmp(name, inputName)) {
                strlcpy(input_name, filename, PATH_MAX);
                break;
            } else {
                close(fd);
                fd = -1;
            }
        }
    }
    closedir(dir);
    ALOGE_IF(fd<0, "couldn't find '%s' input device", inputName);
    return fd;
}

int SensorBase::injectEvents(sensors_event_t*, int)
{
        ALOGW("injectEvents is not implemented");
        return 0;
}
int SensorBase::setLatency(int32_t, int64_t latency_ns)
{
        int fd;
        int latency_ms;
        ssize_t len;
        char buf[80];

        if ((latency_ns / 1000000ULL) >= ((1ULL << 31) - 1))
                return -EINVAL;

        latency_ms = latency_ns / 1000000;
        strlcpy(&input_sysfs_path[input_sysfs_path_len],
                        SYSFS_MAXLATENCY, SYSFS_MAXLEN);
        fd = open(input_sysfs_path, O_RDWR);
        if (fd < 0) {
                ALOGE("open %s failed.(%s)", input_sysfs_path, strerror(errno));
                return -1;
        }

        snprintf(buf, sizeof(buf), "%d", latency_ms);
        len = write(fd, buf, strlen(buf) + 1);
        if (len < (ssize_t)strlen(buf) + 1) {
                ALOGE("write %s:%s failed.(%s)", input_sysfs_path, buf, strerror(errno));
                close(fd);
                return -1;
        }

        close(fd);
        return 0;
}

int SensorBase::flush(int32_t handle)
{
        int fd;
        const char *buf = "1";
        int len;

        NativeSensorManager& sm(NativeSensorManager::getInstance());
        struct SensorContext* ctx = sm.getInfoByHandle(handle);

        /* The SensorService will call
         * batch->flush(not call for the first connection)->activiate
         * Note the number of FLUSH_COMPLETE events should be the
         * same as the number of *flush* called.
         */

        /* Should return -EINVAL if the sensor is not enabled */
        if ((!mEnabled) || (ctx == NULL)) {
                ALOGE("handle:%d mEnabled:%d ctx:%p\n", handle, mEnabled, ctx);
                return -EINVAL;
        }

        /* sensors have FIFO: call into driver */
        if (ctx->sensor->fifoMaxEventCount) {
                strlcpy(&input_sysfs_path[input_sysfs_path_len],
                                SYSFS_FLUSH, SYSFS_MAXLEN);
                fd = open(input_sysfs_path, O_RDWR);
                if (fd < 0) {
                        ALOGE("open %s failed.(%s)", input_sysfs_path, strerror(errno));
                        return -1;
                }

                len = write(fd, buf, strlen(buf)+1);
                if (len < (ssize_t)strlen(buf) + 1) {
                        ALOGE("write %s failed.(%s)", input_sysfs_path, strerror(errno));
                        close(fd);
                        return -1;
                }
                close(fd);
        }

        mHasPendingMetadata++;
        return 0;
}


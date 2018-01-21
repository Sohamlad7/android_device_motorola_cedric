/*
 * Copyright (C) 2016 Motorola Mobility
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

#ifndef BASE_HAL_H
#define BASE_HAL_H

#include <vector>
#include <memory>
#include <cstdint>
#include <cinttypes>
#include <hardware/sensors.h>

#include "SensorBase.h"
#include "SensorsLog.h"


/** This BaseHal class maintains a list of sensor drivers that service this HAL
 * and dispatches the sensor-specific framework calls to the appropriate sensor
 * driver based on the handle (or sensor ID) provided by the framework.
 *
 * Derived classes would typically populate (in their constructor) BaseHal::drivers
 * with a driver for each sensor (or group of sensors) they support, and
 * override poll() and getSensorsList().
 */
class BaseHal {
public:
    /** Since this class contains virtual functions, it is generally NOT
     * compatible with sensors_poll_device_t or sensors_poll_device_1,
     * so it can't be safely down-cast to one of them without using
     * container_of().
     */
    union {
        struct sensors_poll_device_t   v0;
        struct sensors_poll_device_1   v1;
    };

    /** Called by SensorModuleT after the HAL has been constructed. */
    virtual void init() { }
    virtual ~BaseHal() = default;

    /**
     * Enumerate all available sensors. The list is returned in "list".
     * @return number of sensors in the list
     */
    virtual int getSensorsList(struct sensor_t const** list) = 0;

    /**
     *  Place the module in a specific mode. The following modes are defined
     *
     *  0 - Normal operation. Default state of the module.
     *  1 - Loopback mode. Data is injected for the supported
     *      sensors by the sensor service in this mode.
     * @return 0 on success
     *         -EINVAL if requested mode is not supported
     *         -EPERM if operation is not allowed
     */
    virtual int setOperationMode(unsigned int mode) {
        return mode == 0 ? 0 : -EINVAL;
    }

    /// Map sensor handle/id to the corresponding driver
    std::shared_ptr<SensorBase> handleToDriver(int handle) {
        for (const auto& d : drivers) {
            if (d->hasSensor(handle)) return d;
        }

        S_LOGE("No driver for handle %d", handle);
        return nullptr;
    }

    virtual int close() {
        return 0;
    }

    /** Activate/de-activate one sensor. Return 0 on success.
     *
     * @param handle the handle of the sensor to change.
     * @param enabled set to 1 to enable, or 0 to disable the sensor.
     *
     * @return 0 on success, negative errno code otherwise.
     */
    virtual int activate(int handle, int enabled) {
        S_LOGD("+");

        std::shared_ptr<SensorBase> s = handleToDriver(handle);
        if (s == nullptr) return -EINVAL;
        return s->setEnable(handle, enabled);
    }

    /**
     * \brief Implement Android HAL poll()
     *
     * From [source.android.com](https://source.android.com/devices/sensors/hal-interface.html)
     *
     * Returns an array of sensor data by filling the data argument. This function
     * must block until events are available. It will return the number of events
     * read on success, or a negative error number in case of an error.
     *
     * The number of events returned in data must be less or equal to the count
     * argument. This function shall never return 0 (no event).
     *
     * \param[out] data  the returned data items
     * \param[in]  count the maximum number of returned data items
     *
     * \returns negative on failure, the number of returned data items on success,
     *          and never 0.
     */
    virtual int poll(sensors_event_t* data, int count) = 0;

    /**
     * Sets a sensor’s parameters, including sampling frequency and maximum
     * report latency. This function can be called while the sensor is
     * activated, in which case it must not cause any sensor measurements to
     * be lost: transitioning from one sampling rate to the other cannot cause
     * lost events, nor can transitioning from a high maximum report latency to
     * a low maximum report latency.
     * See the Batching sensor results page for details:
     * http://source.android.com/devices/sensors/batching.html
     */
    virtual int batch(int handle, int flags, int64_t ns, int64_t timeout) {
        std::shared_ptr<SensorBase> s = handleToDriver(handle);
        if (s == nullptr) return -EINVAL;
        return s->batch(handle, flags, ns, timeout);
    }

    /**
     * Flush adds a META_DATA_FLUSH_COMPLETE event (sensors_event_meta_data_t)
     * to the end of the "batch mode" FIFO for the specified sensor and flushes
     * the FIFO.
     * If the FIFO is empty or if the sensor doesn't support batching (FIFO size zero),
     * it should return SUCCESS along with a trivial META_DATA_FLUSH_COMPLETE event added to the
     * event stream. This applies to all sensors other than one-shot sensors.
     * If the sensor is a one-shot sensor, flush must return -EINVAL and not generate
     * any flush complete metadata.
     * If the sensor is not active at the time flush() is called, flush() should return
     * -EINVAL.
     */
    virtual int flush(int handle) {
        std::shared_ptr<SensorBase> s = handleToDriver(handle);
        if (s == nullptr) return -EINVAL;
        return s->flush(handle);
    }

protected:
    std::vector<std::shared_ptr<SensorBase>> drivers;
};


template <class HalT>
struct SensorsModuleT : public sensors_module_t {

    SensorsModuleT(const char *name) : name(name) {
        S_LOGD("+ %s", name);

        static_assert(std::is_base_of<BaseHal, HalT>::value, "HalT must inherit from BaseHal");

        if (!getHal()) {
            S_LOGE("No sensor HAL object! About to crash and burn!");
        }

        getHal()->init();

        moduleMethods.open = open;
        this->common = {
            .tag = HARDWARE_MODULE_TAG,
            .version_major = 1,
            .version_minor = 0,
            .id = SENSORS_HARDWARE_MODULE_ID,
            .name = name,
            .author = "Motorola",
            .methods = &moduleMethods,
            .dso = NULL,
            .reserved = {0},
        };
        this->get_sensors_list = getSensorsList;
        this->set_operation_mode = setOperationMode;
    }

    static int getSensorsList(struct sensors_module_t* module, struct sensor_t const** list) {
        (void)module;
        S_LOGD("+");
        return getHal()->getSensorsList(list);
    }

    static int setOperationMode(unsigned int mode) {
        S_LOGD("mode=%d", mode);
        return getHal()->setOperationMode(mode);
    }

    static int open(const struct hw_module_t* module, const char* id, struct hw_device_t** device) {
        (void)id;
        //S_LOGD("module=%08" PRIxPTR, (void *)module);
        std::shared_ptr<HalT> hal = getHal();

        if (!module || !hal) {
            S_LOGE("No module or HAL");
            return -EINVAL;
        }

        // v0.common is a hw_device_t
        hal->v0.common.tag       = HARDWARE_DEVICE_TAG;
        hal->v0.common.version   = SENSORS_DEVICE_API_VERSION_1_3;
        hal->v0.common.module    = const_cast<hw_module_t*>(module);
        hal->v0.common.close     = close;

        hal->v0.activate         = activate;
        hal->v0.setDelay         = setDelay;
        hal->v0.poll             = poll;
        hal->v1.batch            = batch;
        hal->v1.flush            = flush;

        *device = &hal->v0.common;
        return 0;
    }

    static int close(struct hw_device_t *dev) {
        UNUSED(dev);
        return getHal()->close();
    }

    static int activate(struct sensors_poll_device_t *dev, int handle, int enabled) {
        (void)dev;
        S_LOGD("+");
        return getHal()->activate(handle, enabled);
    }

    static int setDelay(struct sensors_poll_device_t *dev, int handle, int64_t ns) {
        S_LOGD("+");
        return batch(reinterpret_cast<sensors_poll_device_1_t *>(dev), handle, 0, ns, 0);
    }

    static int poll(struct sensors_poll_device_t *dev, sensors_event_t* data, int count) {
        (void)dev;
        int ret = 0;
        do {
            ret = getHal()->poll(data, count);
        } while( ret == 0 );
        return ret;
    }

    static int batch(sensors_poll_device_1_t *dev, int handle, int flags, int64_t ns, int64_t timeout) {
        S_LOGD("+");
        (void)dev;
        return getHal()->batch(handle, flags, ns, timeout);
    }

    static int flush(sensors_poll_device_1_t *dev, int handle) {
        (void)dev;
        return getHal()->flush(handle);
    }

private:
    struct hw_module_methods_t moduleMethods;
    const char *name;

    /** The hal can't be made a simple static object due to the undefined
     * static initialization order, so we use a getter instead to guarantee it
     * is constructed on first use. */
    static std::shared_ptr<HalT> getHal() {
        static std::shared_ptr<HalT> hal = std::make_shared<HalT>();
        return hal;
    }
};

#endif // BASE_HAL_H

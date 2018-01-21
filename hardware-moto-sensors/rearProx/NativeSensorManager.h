/*--------------------------------------------------------------------------
Copyright (c) 2014, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------*/

#ifndef SENSOR_INFO_MANAGER_H
#define SENSOR_INFO_MANAGER_H

#include <string.h>
#include <dirent.h>
#include <utils/Log.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <SensorBase.h>

#include <utils/Singleton.h>
#include <cutils/list.h>
#include <sensors.h>
#include <utils/KeyedVector.h>


#include "ProximitySensor.h"
#include "CapSensor.h"
#include "sensors_extension.h"
#include "sensors.h"
using namespace android;

#define EVENT_PATH "/dev/input/"
#define DEPEND_ON(m, t) (m & (1ULL << t))
#define SENSORS_HANDLE(x) (SENSORS_HANDLE_BASE + x + 1)

#ifndef list_for_each_safe
#define list_for_each_safe(node, n, list) \
	for (node = (list)->next, n = node->next; \
		node != (list); \
		node = n, n = node->next)
#endif

enum {
	TYPE_STRING = 0,
	TYPE_INTEGER,
	TYPE_FLOAT,
	TYPE_INTEGER64,
};

struct SensorContext {
	char   name[SYSFS_MAXLEN]; // name of the sensor
	char   vendor[SYSFS_MAXLEN]; // vendor of the sensor
	char   enable_path[PATH_MAX]; // the control path of this sensor
	char   data_path[PATH_MAX]; // the data path to get sensor events

	struct sensor_t *sensor; // point to the sensor_t structure in the sensor list
	SensorBase     *driver; // point to the sensor driver instance

	int data_fd; // the file descriptor of the data device node
	int enable; // indicate if the sensor is enabled
	bool is_virtual; // indicate if this is a virtual sensor
	int64_t delay_ns; // the poll delay setting of this sensor
	int64_t latency_ns; // the max report latency of this sensor
	struct listnode dep_list; // the background sensor type needed for this sensor

	struct listnode listener; // the head of listeners of this sensor
};

struct SensorEventMap {
	char data_name[80];
	char data_path[PATH_MAX];
};

struct SysfsMap {
	int offset;
	const char *node;
	int type;
	int required;
};

/* To contain the listener list and denpend list */
struct SensorRefMap {
	struct listnode list;
	struct SensorContext *ctx;
};

class NativeSensorManager : public Singleton<NativeSensorManager> {
	friend class Singleton<NativeSensorManager>;
	NativeSensorManager();
	~NativeSensorManager();
	struct sensor_t sensor_list[MAX_SENSORS];
	struct SensorContext context[MAX_SENSORS];
	struct SensorEventMap event_list[MAX_SENSORS];
	static const struct SysfsMap node_map[];
	static const struct sensor_t virtualSensorList[];
	static char virtualSensorName[][SYSFS_MAXLEN];

	int mSensorCount;
	bool mScanned;
	int mEventCount;

	DefaultKeyedVector<int32_t, struct SensorContext*> type_map;
	DefaultKeyedVector<int32_t, struct SensorContext*> handle_map;
	DefaultKeyedVector<int, struct SensorContext*> fd_map;

	int getNode(char *buf, char *path, const struct SysfsMap *map);
	int getSensorListInner();
	int getDataInfo();
	int registerListener(struct SensorContext *hw, struct SensorContext *virt);
	int unregisterListener(struct SensorContext *hw, struct SensorContext *virt);
	int syncDelay(int handle);
	int addDependency(struct SensorContext *ctx, int handle);
	int getEventPath(const char *sysfs_path, char *event_path);
	int getEventPathOld(const struct SensorContext *list, char *event_path);
public:
	int getSensorList(const sensor_t **list);
	inline SensorContext* getInfoByFd(int fd) { return fd_map.valueFor(fd); };
	inline SensorContext* getInfoByHandle(int handle) { return handle_map.valueFor(handle); };
	inline SensorContext* getInfoByType(int type) { return type_map.valueFor(type); };
	int getSensorCount() {return mSensorCount;}
	void dump();
	int hasPendingEvents(int handle);
	int activate(int handle, int enable);
	int setDelay(int handle, int64_t ns);
	int syncLatency(int handle);
	int readEvents(int handle, sensors_event_t *data, int count);
	int batch(int handle, int64_t sample_ns, int64_t latency_ns);
	int flush(int handle);
};

#endif


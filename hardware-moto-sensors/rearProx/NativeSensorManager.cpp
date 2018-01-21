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
#include "NativeSensorManager.h"

#define SENSOR_TYPE_MOTO_CAPSENSE SENSOR_TYPE_DEVICE_PRIVATE_BASE + 16

ANDROID_SINGLETON_STATIC_INSTANCE(NativeSensorManager);

int NativeSensorManager::addDependency(struct SensorContext *ctx, int handle)
{
	struct SensorContext *dep;
	struct listnode *node;
	struct SensorRefMap *ref, *item;

	if (!ctx->is_virtual) {
		ALOGE("Only available for virtual sensors.\n");
		return -1;
	}

	dep = getInfoByHandle(handle);

#if defined(SENSORS_DEVICE_API_VERSION_1_3)
	if (ctx->sensor->maxDelay == 0)
		ctx->sensor->maxDelay = dep->sensor->maxDelay;
	else
		ctx->sensor->maxDelay = dep->sensor->maxDelay < ctx->sensor->maxDelay ?
			dep->sensor->maxDelay : ctx->sensor->maxDelay;
#endif

	if (dep != NULL) {
		list_for_each(node, &ctx->dep_list) {
			ref = node_to_item(node, struct SensorRefMap, list);
			if (ref->ctx == dep) {
				ALOGW("The dependency already present");
				return 0;
			}
		}

		item = new SensorRefMap;
		item->ctx = dep;
		list_add_tail(&ctx->dep_list, &item->list);

		return 0;
	}

	return -1;
}

const struct SysfsMap NativeSensorManager::node_map[] = {
	{offsetof(struct sensor_t, name), SYSFS_NAME, TYPE_STRING, 1},
	{offsetof(struct sensor_t, vendor), SYSFS_VENDOR, TYPE_STRING, 1},
	{offsetof(struct sensor_t, version), SYSFS_VERSION, TYPE_INTEGER, 1},
	{offsetof(struct sensor_t, type), SYSFS_TYPE, TYPE_INTEGER, 1},
	{offsetof(struct sensor_t, maxRange), SYSFS_MAXRANGE, TYPE_FLOAT, 1},
	{offsetof(struct sensor_t, resolution), SYSFS_RESOLUTION, TYPE_FLOAT, 1},
	{offsetof(struct sensor_t, power), SYSFS_POWER, TYPE_FLOAT, 1},
	{offsetof(struct sensor_t, minDelay), SYSFS_MINDELAY, TYPE_INTEGER, 1},
	{offsetof(struct sensor_t, fifoReservedEventCount), SYSFS_FIFORESVCNT, TYPE_INTEGER, 0},
	{offsetof(struct sensor_t, fifoMaxEventCount), SYSFS_FIFOMAXCNT, TYPE_INTEGER, 0},
#if defined(SENSORS_DEVICE_API_VERSION_1_3)
#if defined(__LP64__)
	{offsetof(struct sensor_t, maxDelay), SYSFS_MAXDELAY, TYPE_INTEGER64, 0},
	{offsetof(struct sensor_t, flags), SYSFS_FLAGS, TYPE_INTEGER64, 0},
#else
	{offsetof(struct sensor_t, maxDelay), SYSFS_MAXDELAY, TYPE_INTEGER, 0},
	{offsetof(struct sensor_t, flags), SYSFS_FLAGS, TYPE_INTEGER, 0},
#endif
#endif
};

NativeSensorManager::NativeSensorManager():
	mSensorCount(0), mScanned(false), mEventCount(0), type_map(NULL), handle_map(NULL), fd_map(NULL)
{
	int i;

	memset(sensor_list, 0, sizeof(sensor_list));
	memset(context, 0, sizeof(context));

	type_map.setCapacity(MAX_SENSORS);
	handle_map.setCapacity(MAX_SENSORS);
	fd_map.setCapacity(MAX_SENSORS);

	for (i = 0; i < MAX_SENSORS; i++) {
		context[i].sensor = &sensor_list[i];
		sensor_list[i].name = context[i].name;
		sensor_list[i].vendor = context[i].vendor;
		list_init(&context[i].listener);
		list_init(&context[i].dep_list);
	}

	if(getDataInfo()) {
		ALOGE("Get data info failed\n");
	}

	dump();
}

NativeSensorManager::~NativeSensorManager()
{
	int i;
	int number = getSensorCount();
	struct listnode *node;
	struct listnode *n;
	struct SensorContext *ctx;
	struct SensorRefMap *item;

	for (i = 0; i < number; i++) {
		if (context[i].driver != NULL) {
			delete context[i].driver;
		}

		list_for_each_safe(node, n, &context[i].listener) {
			item = node_to_item(node, struct SensorRefMap, list);
			if (item != NULL) {
				list_remove(&item->list);
				delete item;
			}
		}

		list_for_each_safe(node, n, &context[i].dep_list) {
			item = node_to_item(node, struct SensorRefMap, list);
			if (item != NULL) {
				list_remove(&item->list);
				delete item;
			}
		}
	}
}

void NativeSensorManager::dump()
{
	int i;
	struct listnode *node;
	struct SensorRefMap* ref;

	for (i = 0; i < mSensorCount; i++) {
		ALOGI("\nname:%s\ntype:%d\nhandle:%d\ndata_fd=%d\nis_virtual=%d",
				context[i].sensor->name,
				context[i].sensor->type,
				context[i].sensor->handle,
				context[i].data_fd,
				context[i].is_virtual);

		ALOGI("data_path=%s\nenable_path=%s\ndelay_ns:%lld\nenable=%d\n",
				context[i].data_path,
				context[i].enable_path,
				context[i].delay_ns,
				context[i].enable);

#if defined(SENSORS_DEVICE_API_VERSION_1_3)
		ALOGI("minDelay=%d maxDelay=%d flags=%d\n",
				context[i].sensor->minDelay,
				context[i].sensor->maxDelay,
				context[i].sensor->flags);
#endif


		ALOGI("Listener:");
		list_for_each(node, &context[i].listener) {
			ref = node_to_item(node, struct SensorRefMap, list);
			ALOGI("name:%s handle:%d\n", ref->ctx->sensor->name, ref->ctx->sensor->handle);
		}

		ALOGI("Dependency:");
		list_for_each(node, &context[i].dep_list) {
			ref = node_to_item(node, struct SensorRefMap, list);
			ALOGI("name:%s handle:%d", ref->ctx->sensor->name, ref->ctx->sensor->handle);
		}
	}

	ALOGI("\n");
}

int NativeSensorManager::getDataInfo() {
	int i, j;
	struct SensorContext *list;
	int has_proximity = 0;
	struct sensor_t sensor_proximity;

	mSensorCount = getSensorListInner();
	for (i = 0; i < mSensorCount; i++) {
		struct SensorRefMap *item;
		list = &context[i];
		list->is_virtual = false;

		item = new struct SensorRefMap;
		item->ctx = list;
		/* hardware sensor depend on itself */
		list_add_tail(&list->dep_list, &item->list);

		if (strlen(list->data_path) != 0)
			list->data_fd = open(list->data_path, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
		else
			list->data_fd = -1;

		if (list->data_fd > 0) {
			fd_map.add(list->data_fd, list);
		} else {
			ALOGE("open %s failed, continue anyway.(%s)\n", list->data_path, strerror(errno));
		}

		type_map.add(list->sensor->type, list);
		handle_map.add(list->sensor->handle, list);

		switch (list->sensor->type) {
			case SENSOR_TYPE_PROXIMITY:
				has_proximity = 1;
#if defined(SENSORS_DEVICE_API_VERSION_1_3)
				/* reporting mode fix up */
				list->sensor->flags |= SENSOR_FLAG_ON_CHANGE_MODE;
#endif
				list->driver = new ProximitySensor(list);
				sensor_proximity = *(list->sensor);
				break;
			case SENSOR_TYPE_MOTO_CAPSENSE:
				list->sensor->stringType = "com.motorola.sensor.capsense";
#if defined(SENSORS_DEVICE_API_VERSION_1_3)
				/* reporting mode fix up */
				list->sensor->flags |= SENSOR_FLAG_ON_CHANGE_MODE;
#endif
				list->driver = new CapSensor(list);
				break;
			default:
				list->driver = NULL;
				ALOGE("No handle %d for this type sensor!", i);
				break;
		}
	}

	return 0;
}

/* Register a listener on "hw" for "virt".
 * The "hw" specify the actual background sensor type, and "virt" is one kind of virtual sensor.
 * Generally the virtual sensor specified by "virt" can only work when the hardware sensor specified
 * by "hw" is activiated.
 */
int NativeSensorManager::registerListener(struct SensorContext *hw, struct SensorContext *virt)
{
	struct listnode *node;
	struct SensorContext *ctx;
	struct SensorRefMap *item;

	list_for_each(node, &hw->listener) {
		item = node_to_item(node, struct SensorRefMap, list);
		if (item->ctx->sensor->handle == virt->sensor->handle) {
			ALOGE("Already registered as listener for %s:%s\n", hw->sensor->name, virt->sensor->name);
			return -1;
		}
	}

	item = new SensorRefMap;
	item->ctx = virt;

	list_add_tail(&hw->listener, &item->list);

	return 0;
}

/* Remove the virtual sensor listener from the list specified by "hw" */
int NativeSensorManager::unregisterListener(struct SensorContext *hw, struct SensorContext *virt)
{
	struct listnode *node;
	struct listnode *n;
	struct SensorContext *ctx;
	struct SensorRefMap *item;

	list_for_each_safe(node, n, &hw->listener) {
		item = node_to_item(node, struct SensorRefMap, list);
		if (item->ctx == virt) {
			list_remove(&item->list);
			delete item;
			return 0;
		}
	}

	ALOGE("%s is not a listener of %s\n", virt->sensor->name, hw->sensor->name);
	return -1;
}

int NativeSensorManager::getSensorList(const sensor_t **list) {
	*list = mSensorCount ? sensor_list:NULL;

	return mSensorCount;
}

int NativeSensorManager::getNode(char *buf, char *path, const struct SysfsMap *map) {
	char * fret;
	ssize_t len = 0;
	int fd;
	char tmp[SYSFS_MAXLEN];

	if (NULL == buf || NULL == path)
		return -1;

	memset(tmp, 0, sizeof(tmp));

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		ALOGE("open %s failed.(%s)\n", path, strerror(errno));
		/* Ignore unrequired nodes for backward compatiblity */
		return map->required ? -1 : 0;
	}

	len = read(fd, tmp, sizeof(tmp) - 1);
	if ((len <= 0) || (strlen(tmp) == 0)) {
		ALOGE("read %s failed.(%s)\n", path, strerror(errno));
		close(fd);

		/* Ignore unrequired nodes for backward compatiblity */
		return map->required ? -1 : 0;
	}

	tmp[len - 1] = '\0';

	if (tmp[strlen(tmp) - 1] == '\n')
		tmp[strlen(tmp) - 1] = '\0';

	if (map->type == TYPE_INTEGER) {
		int *p = (int *)(buf + map->offset);
		*p = atoi(tmp);
	} else if (map->type == TYPE_STRING) {
		char **p = (char **)(buf + map->offset);
		strlcpy(*p, tmp, SYSFS_MAXLEN);
	} else if (map->type == TYPE_FLOAT) {
		float *p = (float*)(buf + map->offset);
		*p = atof(tmp);
	} else if (map->type == TYPE_INTEGER64) {
		int64_t *p = (int64_t *)(buf + map->offset);
		*p = atoll(tmp);
	}

	close(fd);
	return 0;
}

int NativeSensorManager::getEventPathOld(const struct SensorContext *list, char *event_path)
{
	struct dirent **namelist;
	char *file;
	char path[PATH_MAX];
	char name[80];
	int nNodes;
	int fd = -1;
	int j;

	/* scan "/dev/input" to get information */
	if (!mScanned) {
		strlcpy(path, EVENT_PATH, sizeof(path));
		file = path + strlen(EVENT_PATH);
		nNodes = scandir(path, &namelist, 0, alphasort);
		if (nNodes < 0) {
			ALOGE("scan %s failed.(%s)\n", EVENT_PATH, strerror(errno));
			return -1;
		}

		for (mEventCount = 0, j = 0; (j < nNodes) && (j < MAX_SENSORS); j++) {
			if (namelist[j]->d_type != DT_CHR) {
				continue;
			}

			strlcpy(file, namelist[j]->d_name, sizeof(path) - strlen(EVENT_PATH));

			fd = open(path, O_RDONLY);
			if (fd < 0) {
				ALOGE("open %s failed(%s)", path, strerror(errno));
				continue;
			}

			if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
				name[0] = '\0';
			}

			strlcpy(event_list[mEventCount].data_name, name, sizeof(event_list[0].data_name));
			strlcpy(event_list[mEventCount].data_path, path, sizeof(event_list[0].data_path));
			close(fd);
			mEventCount++;
		}

		for (j = 0; j <nNodes; j++ ) {
			free(namelist[j]);
		}

		free(namelist);
		mScanned = true;
	}

	/* Initialize data_path and data_fd */
	for (j = 0; (j < mEventCount) && (j < MAX_SENSORS); j++) {
		if (strcmp(list->sensor->name, event_list[j].data_name) == 0) {
			strlcpy(event_path, event_list[j].data_path, PATH_MAX);
			break;
		}

		if (strcmp(event_list[j].data_name, type_to_name(list->sensor->type)) == 0) {
			strlcpy(event_path, event_list[j].data_path, PATH_MAX);
		}
	}

	return 0;
}

int NativeSensorManager::getEventPath(const char *sysfs_path, char *event_path)
{
	DIR *dir;
	struct dirent *de;
	char symlink[PATH_MAX];
	int len;
	char *needle;

	dir = opendir(sysfs_path);
	if (dir == NULL) {
		ALOGE("open %s failed.(%s)\n", sysfs_path,strerror(errno));
		return -1;
	}
	if ((sysfs_path == NULL) || (event_path == NULL)) {
		ALOGE("invalid NULL argument.");
		return -EINVAL;
	}

	len = readlink(sysfs_path, symlink, PATH_MAX);
	if (len < 0) {
		ALOGE("readlink failed for %s(%s)\n", sysfs_path, strerror(errno));
		return -1;
	}

	needle = strrchr(symlink, '/');
	if (needle == NULL) {
		ALOGE("unexpected symlink %s\n", symlink);
		return -1;
	}

	if (strncmp(needle + 1, "input", strlen("input")) != 0) {
		ALOGE("\n");
		ALOGE("==========================Notice=================================");
		ALOGE("sensors_classdev %s need to register as the child of input device\n", sysfs_path);
		ALOGE("in order to speed up Android sensor service initialization time");
		ALOGE("Please update your sensor driver.");
		ALOGE("================================================================");
		ALOGE("\n");

		return -ENODEV;
	}

	strlcpy(event_path, EVENT_PATH, PATH_MAX);

	while ((de = readdir(dir))) {
		if (strncmp(de->d_name, "event", strlen("event")) == 0) {
			strlcat(event_path, de->d_name, sizeof(de->d_name));
			break;
		}
	}

	closedir(dir);

	return 0;
}

int NativeSensorManager::getSensorListInner()
{
	int number = 0;
	int err = -1;
	const char *dirname = SYSFS_CLASS;
	char devname[PATH_MAX];
	char *filename;
	char *nodename;
	DIR *dir;
	struct dirent *de;
	struct SensorContext *list;
	unsigned int i;

	dir = opendir(dirname);
	if(dir == NULL) {
		return 0;
	}
	strlcpy(devname, dirname, PATH_MAX);
	filename = devname + strlen(devname);

	while ((de = readdir(dir))) {
		if(de->d_name[0] == '.' &&
			(de->d_name[1] == '\0' ||
				(de->d_name[1] == '.' && de->d_name[2] == '\0')))
			continue;

		list = &context[number];

		strlcpy(filename, de->d_name, PATH_MAX - strlen(SYSFS_CLASS));
		nodename = filename + strlen(de->d_name);
		*nodename++ = '/';

		for (i = 0; i < ARRAY_SIZE(node_map); i++) {
			strlcpy(nodename, node_map[i].node, PATH_MAX - strlen(SYSFS_CLASS) - strlen(de->d_name));
			err = getNode((char*)(list->sensor), devname, &node_map[i]);
			if (err) {
				ALOGE("Get node for %s failed.\n", devname);
				break;
			}
		}

		if (i < ARRAY_SIZE(node_map))
			continue;

		//if (!((1ULL << list->sensor->type) & SUPPORTED_SENSORS_TYPE)){
		if (!is_sensor_supported(list->sensor->type)){
			ALOGE("not suport type:%d", list->sensor->type);
			continue;
		}
		/* Setup other information */
#if defined(SENSORS_DEVICE_API_VERSION_1_3)
		if (list->sensor->maxDelay == 0)
			list->sensor->maxDelay = 10000000;
		else
			list->sensor->maxDelay = list->sensor->maxDelay * 1000; /* milliseconds to microseconds */
#endif
		list->sensor->handle = SENSORS_HANDLE(number);

		strlcpy(nodename, "", SYSFS_MAXLEN);
		strlcpy(list->enable_path, devname, PATH_MAX);

		/* initialize data path */
		strlcpy(nodename, "device", SYSFS_MAXLEN);

		if (getEventPath(devname, list->data_path) == -ENODEV) {
			getEventPathOld(list, list->data_path);
		}

		number++;
	}
	closedir(dir);
	return number;
}

int NativeSensorManager::activate(int handle, int enable)
{
	SensorContext *list;
	int i;
	int number = getSensorCount();
	int err = 0;
	struct listnode *node;
	struct SensorContext *ctx;
	struct SensorRefMap *item;

	ALOGD("activate called handle:%d enable:%d", handle, enable);

	list = getInfoByHandle(handle);
	if (list == NULL) {
		ALOGE("Invalid handle(%d)", handle);
		return -EINVAL;
	}

	list->enable = enable;

	/* Search for the background sensor for the sensor specified by handle. */
	list_for_each(node, &list->dep_list) {
		item = node_to_item(node, struct SensorRefMap, list);
		if (enable) {
			registerListener(item->ctx, list);

#if defined(SENSORS_DEVICE_API_VERSION_1_3)
			/* HAL 1.3 already set listener's delay and latency
			 * Sync it right now to make it take effect.
			 */
			syncDelay(item->ctx->sensor->handle);
			syncLatency(item->ctx->sensor->handle);
#endif

			/* Enable the background sensor and register a listener on it. */
			ALOGD("%s calling driver enable", item->ctx->sensor->name);
			item->ctx->driver->enable(item->ctx->sensor->handle, 1);

		} else {
			/* The background sensor has other listeners, we need
			 * to unregister the current sensor from it and sync the
			 * poll delay settings.
			 */
			if (!list_empty(&item->ctx->listener)) {
				unregisterListener(item->ctx, list);
				/* restore delay settings */
				syncDelay(item->ctx->sensor->handle);

#if defined(SENSORS_DEVICE_API_VERSION_1_3)
				/* restore latency settings */
				syncLatency(item->ctx->sensor->handle);
#endif
			}

			/* Disable the background sensor if it doesn't have any listeners. */
			if (list_empty(&item->ctx->listener)) {
				ALOGD("%s calling driver disable", item->ctx->sensor->name);
				item->ctx->driver->enable(item->ctx->sensor->handle, 0);
			}

		}
	}

	/* Settings change notification */
	if (list->is_virtual) {
		ALOGD("%s calling driver %s", list->sensor->name, enable ? "enable" : "disable");
		list->driver->enable(handle, enable);
	}

	return err;
}

int NativeSensorManager::syncDelay(int handle)
{
	const SensorRefMap *item;
	SensorContext *ctx;
	const SensorContext *list;
	struct listnode *node;
	int64_t min_ns;

	list = getInfoByHandle(handle);
	if (list == NULL) {
		ALOGE("Invalid handle(%d)", handle);
		return -EINVAL;
	}

	if (list_empty(&list->listener))
		return 0;

	node = list_head(&list->listener);
	item = node_to_item(node, struct SensorRefMap, list);
	min_ns = item->ctx->delay_ns;

	list_for_each(node, &list->listener) {
		item = node_to_item(node, struct SensorRefMap, list);
		ctx = item->ctx;
		/* To handle some special case that the polling delay is 0. This
		 * may happen if the background sensor is not enabled but the virtual
		 * sensor is enabled case.
		 */
		if (ctx->delay_ns == 0) {
			ALOGD("%s delay is 0. continue...", ctx->sensor->name);
			continue;
		}

		if (min_ns > ctx->delay_ns)
			min_ns = ctx->delay_ns;
	}

	ALOGD("%s calling driver setDelay %lld ms\n", list->sensor->name, min_ns / 1000000);
	return list->driver->setDelay(list->sensor->handle, min_ns);
}

int NativeSensorManager::syncLatency(int handle)
{
	const SensorRefMap *item;
	SensorContext *ctx;
	const SensorContext *list;
	struct listnode *node;
	int64_t min_ns;

	list = getInfoByHandle(handle);
	if (list == NULL) {
		ALOGE("Invalid handle(%d)", handle);
		return -EINVAL;
	}

	if (list_empty(&list->listener))
		return 0;

	node = list_head(&list->listener);
	item = node_to_item(node, struct SensorRefMap, list);
	min_ns = item->ctx->latency_ns;

	list_for_each(node, &list->listener) {
		item = node_to_item(node, struct SensorRefMap, list);
		ctx = item->ctx;

		if (min_ns > ctx->latency_ns)
			min_ns = ctx->latency_ns;
	}

	if (list->sensor->fifoMaxEventCount) {
		ALOGD("%s calling driver setLatency %lld ms\n", list->sensor->name, min_ns / 1000000);
		list->driver->setLatency(list->sensor->handle, min_ns);
	}

	return 0;
}

int NativeSensorManager::setDelay(int handle, int64_t ns)
{
	SensorContext *list;
	int i;
	int64_t delay = ns;
	struct SensorRefMap *item;
	struct listnode *node;

	ALOGD("setDelay called handle:%d sample_ns:%lld", handle, ns);

	list = getInfoByHandle(handle);
	if (list == NULL) {
		ALOGE("Invalid handle(%d)", handle);
		return -EINVAL;
	}

	if (ns == 0) {
		ALOGE("%s delay set to 0", list->sensor->name);
		return -EINVAL;
	}

	if (ns < list->sensor->minDelay * 1000) {
		ALOGW("%s delay is less than minDelay. Cast it to minDelay", list->sensor->name);
		list->delay_ns = list->sensor->minDelay * 1000;
	} else {
		list->delay_ns = delay;
	}

	list_for_each(node, &list->dep_list) {
		item = node_to_item(node, struct SensorRefMap, list);
		syncDelay(item->ctx->sensor->handle);
	}

	return 0;
}

int NativeSensorManager::readEvents(int handle, sensors_event_t* data, int count)
{
	const SensorContext *list;
	int i, j;
	int number = getSensorCount();
	int nb;
	struct listnode *node;
	struct SensorRefMap *item;

	list = getInfoByHandle(handle);
	if (list == NULL) {
		ALOGE("Invalid handle(%d)", handle);
		return -EINVAL;
	}
	do {
		nb = list->driver->readEvents(data, count);
	} while ((nb == -EAGAIN) || (nb == -EINTR));

	for (j = 0; j < nb; j++) {
		list_for_each(node, &list->listener) {
			item = node_to_item(node, struct SensorRefMap, list);
			if (item->ctx->enable && (item->ctx != list)) {
				item->ctx->driver->injectEvents(&data[j], 1);
			}
		}
	}

	if (list->enable)
		return nb;

	/* No need to report the events if the sensor is not enabled */
	return 0;
}

int NativeSensorManager::batch(int handle, int64_t sample_ns, int64_t latency_ns)
{
	SensorContext *list;
	struct listnode *node;
	struct SensorRefMap *item;

	ALOGD("batch called handle:%d sample_ns:%lld latency_ns:%lld", handle, sample_ns, latency_ns);

	if ((latency_ns != 0) && (latency_ns < sample_ns)) {
		ALOGE("latency_ns is smaller than sample_ns");
		return -EINVAL;
	}

	list = getInfoByHandle(handle);
	if (list == NULL) {
		ALOGE("Invalid handle(%d)", handle);
		return -EINVAL;
	}

	/* *sample_ns* is the same as *ns* passed to setDelay */
	list->delay_ns = sample_ns;
	list->latency_ns = latency_ns;

	/* should take effect now for ones with listeners */
	list_for_each(node, &list->dep_list) {
		item = node_to_item(node, struct SensorRefMap, list);
		syncDelay(item->ctx->sensor->handle);
		syncLatency(item->ctx->sensor->handle);
	}

	return 0;
}

int NativeSensorManager::flush(int handle)
{
	const SensorContext *list;
	int ret = 0;
	struct SensorRefMap *item;
	struct listnode *node;

	ALOGD("flush called %d\n", handle);
	list = getInfoByHandle(handle);
	if (list == NULL) {
		ALOGE("Invalid handle(%d)", handle);
		return -EINVAL;
	}

	list_for_each(node, &list->dep_list) {
		item = node_to_item(node, struct SensorRefMap, list);
		ret = item->ctx->driver->flush(item->ctx->sensor->handle);
		if (ret) {
			ALOGE("Calling flush failed(%d)", ret);
			return ret;
		}
	}

	/* calling flush for virtual sensor */
	if (list->is_virtual) {
		ret = list->driver->flush(handle);
		if (ret) {
			ALOGE("Calling flush failed(%d)", ret);
			return ret;
		}
	}

	return 0;
}

int NativeSensorManager::hasPendingEvents(int handle)
{
	const SensorContext *list;

	list = getInfoByHandle(handle);
	if (list == NULL) {
		ALOGE("Invalid handle(%d)", handle);
		return -EINVAL;
	}

	return list->driver->hasPendingEvents();
}


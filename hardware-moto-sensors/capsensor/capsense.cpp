#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/user.h> // for PAGE_SIZE
#include <hardware_legacy/uevent.h>
#include <cutils/klog.h>

#define LOG_TAG "hsj-listen"
#define CAPRESET "/sys/class/capsense/reset"
#define HSJ_EV "change@/devices/virtual/switch/h2w"

void uevent_handler(void *data, const char *msg, int msg_len);

int main(void)
{
	static char uevent_data[PAGE_SIZE];
	int len;

	uevent_data[PAGE_SIZE-1] = '\0';

	uevent_init();

	// never returns
	while (1) {
		len = uevent_next_event(uevent_data, (int)(sizeof(uevent_data)) - 1);
		if(!strncmp(uevent_data, HSJ_EV, len)) {
			int fd = open(CAPRESET, O_WRONLY);
			if (fd < 0) {
				KLOG_ERROR(LOG_TAG, "Can't open capreset, %s\n", strerror(errno));
				continue;
			}
			len = TEMP_FAILURE_RETRY(write(fd, "1", 1));
			if (len < 1) {
				KLOG_ERROR(LOG_TAG, "Can't write capreset, %s\n", strerror(errno));
			}
			close(fd);
		}
	}

	return 0;
}

#include <cutils/log.h>

// Sensor log macros that include the function name and line number
#define S_LOGE(format, x...) ALOGE("%s:%d " format, __func__, __LINE__, ##x)
#define S_LOGW(format, x...) ALOGW("%s:%d " format, __func__, __LINE__, ##x)
#define S_LOGI(format, x...) ALOGI("%s:%d " format, __func__, __LINE__, ##x)
#define S_LOGD(format, x...) ALOGD("%s:%d " format, __func__, __LINE__, ##x)
#define S_LOGV(format, x...) ALOGV("%s:%d " format, __func__, __LINE__, ##x)


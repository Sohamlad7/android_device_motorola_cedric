#include <stdint.h>

namespace android {
    extern "C" void _ZN7android9StopWatchC1EPKci(const char* name, int clock);

    extern "C" void _ZN7android10frameworks13sensorservice4V1_08toStringENS2_6ResultE() {}
    extern "C" void _ZN7android9StopWatchC1EPKcij(const char* name, int clock, uint32_t flags) {
        _ZN7android9StopWatchC1EPKci(name, clock);
    }
}

#include <utils/String8.h>

namespace android {
    extern "C" void _ZN7android12ConsumerBase7setNameERKNS_7String8E(const String8& name);
    extern "C" void _ZN7android9StopWatchC1EPKci(const char* name, int clock);

    extern "C" void _ZN7android5FenceD1Ev() {}
    extern "C" void _ZN7android10GLConsumer7setNameERKNS_7String8E(const String8& name) {
        _ZN7android12ConsumerBase7setNameERKNS_7String8E(name);
    }
    extern "C" void _ZN7android9StopWatchC1EPKcij(const char* name, int clock, uint32_t flags) {
        _ZN7android9StopWatchC1EPKci(name, clock);
    }
}

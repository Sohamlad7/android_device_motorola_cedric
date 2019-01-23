/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <health2/service.h>
#include <healthd/healthd.h>
#include <hidl/HidlTransportSupport.h>
#include <android-base/file.h>
#include <android-base/strings.h>
#include <android-base/logging.h>
#include <vector>
#include <string>
#include <sys/stat.h>

#include "CycleCountBackupRestore.h"
#include "LearnedCapacityBackupRestore.h"

using ::device::xiaomi::mido::health::CycleCountBackupRestore;
using ::device::xiaomi::mido::health::LearnedCapacityBackupRestore;

static constexpr int kBackupTrigger = 20;

static CycleCountBackupRestore ccBackupRestore;
static LearnedCapacityBackupRestore lcBackupRestore;

 int cycle_count_backup(int battery_level)
{
    static int saved_soc = 0;
    static int soc_inc = 0;
    static bool is_first = true;
     if (is_first) {
        is_first = false;
        saved_soc = battery_level;
        return 0;
    }
     if (battery_level > saved_soc) {
        soc_inc += battery_level - saved_soc;
    }
     saved_soc = battery_level;
     if (soc_inc >= kBackupTrigger) {
        ccBackupRestore.Backup();
        soc_inc = 0;
    }
    return 0;
}

// See : hardware/interfaces/health/2.0/README
void healthd_board_init(struct healthd_config*) 
{
    ccBackupRestore.Restore();
    lcBackupRestore.Restore();
}

int healthd_board_battery_update(struct android::BatteryProperties *props)
{
    cycle_count_backup(props->batteryLevel);
    lcBackupRestore.Backup();
    return 0;
}

int main() {
    return health_service_main();
}

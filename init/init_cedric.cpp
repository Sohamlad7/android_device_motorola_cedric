/*
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
 */

#include <stdlib.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#include <sys/sysinfo.h>
#include <android-base/properties.h>
#include "property_service.h"
#include "vendor_init.h"

using android::init::property_set;

void property_override(char const prop[], char const value[])
{
    prop_info *pi;

    pi = (prop_info*) __system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void property_override_dual(char const system_prop[], char const vendor_prop[],
    char const value[])
{
    property_override(system_prop, value);
    property_override(vendor_prop, value);
}

/* Get Ram size for different variants */
void check_device()
{
    struct sysinfo sys;
    sysinfo(&sys);
    if (sys.totalram > 2048ull * 1024 * 1024) {
        property_set("ro.boot.ram", "3GB");
    } else {
        property_set("ro.boot.ram", "2GB");
    }
}

void num_sims() {
    std::string dualsim;

    dualsim = android::base::GetProperty("ro.boot.dualsim", "");
    property_set("ro.hw.dualsim", dualsim.c_str());

    if (dualsim == "true") {
        property_set("persist.radio.multisim.config", "dsds");
    } else {
        property_set("persist.radio.multisim.config", "");
    }
}

void vendor_load_properties()
{
    std::string platform = android::base::GetProperty("ro.board.platform", "");

    if (platform != ANDROID_TARGET)
        return;

    // camera (treble)
    property_override("ro.product.manufacturer", "motorola");
    property_override("ro.build.product", "cedric");
    property_override("ro.product.device", "cedric");
    property_override("ro.product.name", "cedric_retail");

    // sku
    std::string sku = android::base::GetProperty("ro.boot.hardware.sku", "");
    property_override_dual("ro.product.model", "ro.vendor.product.model", sku.c_str());

    // fingerprint
    property_override("ro.build.description", "cedric-user 8.1.0 OPPS28.85-13-2 d04a4 release-keys");
    property_override_dual("ro.build.fingerprint", "ro.vendor.build.fingerprint", "motorola/cedric/cedric:8.1.0/OPPS28.85-13-2/d04a4:user/release-keys");

    // rmt_storage
    std::string device = android::base::GetProperty("ro.boot.device", "");
    std::string radio = android::base::GetProperty("ro.boot.radio", "");
    property_set("ro.vendor.hw.device", device.c_str());
    property_set("ro.vendor.hw.radio", radio.c_str());
    property_set("ro.hw.fps", "true");
    property_set("ro.hw.imager", "13MP");

    num_sims();

}

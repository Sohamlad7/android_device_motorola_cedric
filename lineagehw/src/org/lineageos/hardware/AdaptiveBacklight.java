/*
 * Copyright (C) 2013 The CyanogenMod Project
 * Copyright (C) 2018 The LineageOS Project
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

package org.lineageos.hardware;

import android.util.Log;

import org.lineageos.internal.util.FileUtils;
import vendor.lineage.livedisplay.V1_0.Feature;

/**
 * Adaptive backlight support (this refers to technologies like NVIDIA SmartDimmer,
 * QCOM CABL or Samsung CABC).
 */
public class AdaptiveBacklight {

    private static final String TAG = "AdaptiveBacklight";

    private static final String FILE_CABC = "/sys/class/graphics/fb0/cabc";

    private static final boolean sHasNativeSupport =
            LiveDisplayVendorImpl.getInstance().hasNativeFeature(Feature.ADAPTIVE_BACKLIGHT);

    /**
     * Whether device supports an adaptive backlight technology.
     *
     * @return boolean Supported devices must return always true
     */
    public static boolean isSupported() {
        if (sHasNativeSupport) {
            return true;
        }

        return FileUtils.isFileWritable(FILE_CABC);
    }

    /**
     * This method return the current activation status of the adaptive backlight technology.
     *
     * @return boolean Must be false when adaptive backlight is not supported or not activated, or
     * the operation failed while reading the status; true in any other case.
     */
    public static boolean isEnabled() {
        try {
            if (sHasNativeSupport) {
                return LiveDisplayVendorImpl.getInstance().isAdaptiveBacklightEnabled();
            }
            return Integer.parseInt(FileUtils.readOneLine(FILE_CABC)) > 0;
        } catch (Exception e) {
            Log.e(TAG, e.getMessage(), e);
        }
        return false;
    }

    /**
     * This method allows to setup adaptive backlight technology status.
     *
     * @param status The new adaptive backlight status
     * @return boolean Must be false if adaptive backlight is not supported or the operation
     * failed; true in any other case.
     */
    public static boolean setEnabled(boolean status) {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.getInstance().setAdaptiveBacklightEnabled(status);
        }
        return FileUtils.writeLine(FILE_CABC, status ? "1" : "0");
    }
}

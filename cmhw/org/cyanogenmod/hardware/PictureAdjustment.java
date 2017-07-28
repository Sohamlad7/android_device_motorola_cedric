/*
 * Copyright (C) 2016 The CyanogenMod Project
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

package org.cyanogenmod.hardware;

import android.util.Range;

import cyanogenmod.hardware.HSIC;

/**
 * Color balance support
 *
 * Color balance controls allow direct adjustment of display color temperature
 * using a range of values. A zero implies no adjustment, negative values
 * move towards warmer temperatures, and positive values move towards
 * cool temperatures.
 */
public class PictureAdjustment {

    private static final boolean sHasNativeSupport =
            LiveDisplayVendorImpl.hasNativeFeature(LiveDisplayVendorImpl.PICTURE_ADJUSTMENT);

    /**
     * Whether device supports color balance control
     *
     * @return boolean Supported devices must return always true
     */
    public static boolean isSupported() {
        return sHasNativeSupport;
    }

    /**
     * This method returns the current picture adjustment values
     *
     * @return HSIC
     */
    public static HSIC getHSIC() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.native_getPictureAdjustment();
        }
        return null;
    }

    /**
     * This method returns the default picture adjustment for the current mode
     *
     * @return HSIC
     */
    public static HSIC getDefaultHSIC() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.native_getDefaultPictureAdjustment();
        }
        return null;
    }

    /**
     * This method allows to set the picture adjustment
     *
     * @param hsic
     * @return boolean Must be false if feature is not supported or the operation
     * failed; true in any other case.
     */
    public static boolean setHSIC(final HSIC hsic) {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.native_setPictureAdjustment(hsic);
        }
        return false;
    }

    /**
     * Get the range available for hue adjustment
     * @return range
     */
    public static Range<Float> getHueRange() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.native_getHueRange();
        }
        return new Range(0.0f, 0.0f);
    }

    /**
     * Get the range available for saturation adjustment
     * @return range
     */
    public static Range<Float> getSaturationRange() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.native_getSaturationRange();
        }
        return new Range(0.0f, 0.0f);
    }

    /**
     * Get the range available for intensity adjustment
     * @return range
     */
    public static Range<Float> getIntensityRange() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.native_getIntensityRange();
        }
        return new Range(0.0f, 0.0f);
    }

    /**
     * Get the range available for contrast adjustment
     * @return range
     */
    public static Range<Float> getContrastRange() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.native_getContrastRange();
        }
        return new Range(0.0f, 0.0f);
    }

    /**
     * Get the range available for saturation threshold adjustment
     *
     * This is an adjustable lower limit where the image is fully
     * desaturated.
     *
     * @return range
     */
    public static Range<Float> getSaturationThresholdRange() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.native_getSaturationThresholdRange();
        }
        return new Range(0.0f, 0.0f);
    }
}

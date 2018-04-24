/*
 * Copyright (C) 2016 The CyanogenMod Project
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

import android.util.Range;

import lineageos.hardware.HSIC;
import vendor.lineage.livedisplay.V1_0.Feature;

/**
 * Picture adjustment support
 *
 * Allows tuning of hue, saturation, intensity, and contrast levels
 * of the display
 */
public class PictureAdjustment {

    private static final boolean sHasNativeSupport =
            LiveDisplayVendorImpl.getInstance().hasNativeFeature(Feature.PICTURE_ADJUSTMENT);

    /**
     * Whether device supports picture adjustment
     *
     * @return boolean Supported devices must return always true
     */
    public static boolean isSupported() {
        return sHasNativeSupport;
    }

    /**
     * This method returns the current picture adjustment values based
     * on the selected DisplayMode.
     *
     * @return the HSIC object or null if not supported
     */
    public static HSIC getHSIC() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.getInstance().getPictureAdjustment();
        }
        return null;
    }

    /**
     * This method returns the default picture adjustment values.
     *
     * If DisplayModes are available, this may change depending on the
     * selected mode.
     *
     * @return the HSIC object or null if not supported
     */
    public static HSIC getDefaultHSIC() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.getInstance().getDefaultPictureAdjustment();
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
            return LiveDisplayVendorImpl.getInstance().setPictureAdjustment(hsic);
        }
        return false;
    }

    /**
     * Get the range available for hue adjustment
     * @return range of floats
     */
    public static Range<Float> getHueRange() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.getInstance().getHueRange();
        }
        return new Range(0.0f, 0.0f);
    }

    /**
     * Get the range available for saturation adjustment
     * @return range of floats
     */
    public static Range<Float> getSaturationRange() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.getInstance().getSaturationRange();
        }
        return new Range(0.0f, 0.0f);
    }

    /**
     * Get the range available for intensity adjustment
     * @return range of floats
     */
    public static Range<Float> getIntensityRange() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.getInstance().getIntensityRange();
        }
        return new Range(0.0f, 0.0f);
    }

    /**
     * Get the range available for contrast adjustment
     * @return range of floats
     */
    public static Range<Float> getContrastRange() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.getInstance().getContrastRange();
        }
        return new Range(0.0f, 0.0f);
    }

    /**
     * Get the range available for saturation threshold adjustment
     *
     * This is the threshold where the display becomes fully saturated
     *
     * @return range of floats
     */
    public static Range<Float> getSaturationThresholdRange() {
        if (sHasNativeSupport) {
            return LiveDisplayVendorImpl.getInstance().getSaturationThresholdRange();
        }
        return new Range(0.0f, 0.0f);
    }
}

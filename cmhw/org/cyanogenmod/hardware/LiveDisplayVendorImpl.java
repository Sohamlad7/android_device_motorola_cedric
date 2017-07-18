/*
 * Copyright (C) 2015 The CyanogenMod Project
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

import android.util.Log;
import android.util.Range;

import cyanogenmod.hardware.DisplayMode;
import cyanogenmod.hardware.HSIC;

/**
 * This class loads an implementation of the LiveDisplay native interface.
 */
public class LiveDisplayVendorImpl {

    public static final String TAG = "LiveDisplayVendorImpl";

    public static final int DISPLAY_MODES = 0x1;
    public static final int COLOR_BALANCE = 0x2;
    public static final int OUTDOOR_MODE = 0x4;
    public static final int ADAPTIVE_BACKLIGHT = 0x8;
    public static final int PICTURE_ADJUSTMENT = 0x10;

    private static boolean sNativeLibraryLoaded;
    private static int     sFeatures;

    static {
        try {
            System.loadLibrary("jni_livedisplay");

            final int features = native_getSupportedFeatures();
            if (features > 0) {
                Log.i(TAG, "Using native LiveDisplay backend (features: " + sFeatures + ")");
            }

            sNativeLibraryLoaded = features > 0;
            sFeatures = features;
        } catch (Throwable t) {
            sNativeLibraryLoaded = false;
            sFeatures = 0;
        }
    }

    public static boolean hasNativeFeature(int feature) {
        return sNativeLibraryLoaded && ((sFeatures & feature) != 0);
    }

    private static native int native_getSupportedFeatures();

    public static native DisplayMode[] native_getDisplayModes();
    public static native DisplayMode native_getCurrentDisplayMode();
    public static native DisplayMode native_getDefaultDisplayMode();
    public static native boolean native_setDisplayMode(DisplayMode mode, boolean makeDefault);

    public static native boolean native_setAdaptiveBacklightEnabled(boolean enabled);
    public static native boolean native_isAdaptiveBacklightEnabled();

    public static native boolean native_setOutdoorModeEnabled(boolean enabled);
    public static native boolean native_isOutdoorModeEnabled();

    public static native Range<Integer> native_getColorBalanceRange();
    public static native int native_getColorBalance();
    public static native boolean native_setColorBalance(int value);

    public static native boolean native_setPictureAdjustment(final HSIC hsic);
    public static native HSIC native_getPictureAdjustment();
    public static native HSIC native_getDefaultPictureAdjustment();

    public static native Range<Float> native_getHueRange();
    public static native Range<Float> native_getSaturationRange();
    public static native Range<Float> native_getIntensityRange();
    public static native Range<Float> native_getContrastRange();
    public static native Range<Float> native_getSaturationThresholdRange();
}

/*
 * Copyright (C) 2013 The CyanogenMod Project
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

/*
 * Display gamma calibration
 *
 * Exports methods to get the valid value boundaries, the
 * number of available RGB triplets, the current color values (per triplet id)
 * and a method to set new ones (also per triplet).
 *
 * Values exported by min/max can be the direct values required
 * by the hardware, or a local (to DisplayGammaCalibration) abstraction
 * that's internally converted to something else prior to actual use. The
 * Settings user interface will present the raw values
 *
 *
 */

public class DisplayGammaCalibration {

    /*
     * All HAF classes should export this boolean.
     * Real implementations must, of course, return true
     */

    public static boolean isSupported() { return false; }

    /*
     * How many RGB triplets does the device export?
     *
     * The most common case here should be 1, but some hardware
     * has multiplier combos
     */

    public static int getNumberOfControls() {
        return 0;
    }

    /*
     * Set the RGB values to the given input triplet on this control
     * index. Input is expected to consist of a zero-indexed control
     * id, and a string containing 3 values, space-separated, each of
     * those a value between the boundaries set by get(Max|Min)Value
     * (see below), and it's meant to be locally interpreted/used.
     */

    public static boolean setGamma(int controlIdx, String gamma) {
        throw new UnsupportedOperationException();
    }

    /*
     * What's the control specific maximum integer value
     * we take for a color
     */

    public static int getMaxValue(int controlIdx) {
        return -1;
    }

    /*
     * What's the control specific minimum integer value
     * we take for a color
     */

    public static int getMinValue(int controlIdx) {
        return -1;
    }

    /*
     * What's the current RGB triplet for this control?
     * This should return a space-separated set of integers in
     * a string, same format as the input to setColors()
     */

    public static String getCurGamma(int controlIdx) {
        return "0 0 0";
    }
}

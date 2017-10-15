/*
 * Copyright (C) 2016 The CyanogenMod Project
 * Copyright (C) 2017 The LineageOS Project
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

package org.lineageos.settings.device.actions;

import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

import org.lineageos.internal.util.FileUtils;

public class Constants {

    private static final String TAG = "LineageActions";

    // Swap keys
    public static final String FP_HOME_KEY = "fp_home";
    public static final String FP_HOME_KEY_OFF = "fp_home_scr_off";

    // Swap nodes
    public static final String FP_HOME_NODE = "/sys/homebutton/enable";
    public static final String FP_HOME_OFF_NODE = "/sys/homebutton/enable_off";

    // Haptic node
    public static final String FP_HAPTIC_NODE = "/sys/homebutton/haptic";
    public static final String FP_HAPTIC_KEY = "fp_haptic";

    // List of keys
    public static final String FP_KEYS = "fp_keys";
    public static final String FP_KEY_HOLD = "fp_key_hold";
    public static final String FP_KEY_LEFT = "fp_key_left";
    public static final String FP_KEY_RIGHT = "fp_key_right";

    public static final String FP_KEYS_OFF = "fp_keys_off";
    public static final String FP_KEY_HOLD_OFF = "fp_key_hold_off";
    public static final String FP_KEY_LEFT_OFF = "fp_key_left_off";
    public static final String FP_KEY_RIGHT_OFF = "fp_key_right_off";

    // Keys nodes
    public static final String FP_KEYS_NODE = "/sys/homebutton/key";
    public static final String FP_KEY_HOLD_NODE = "/sys/homebutton/key_hold";
    public static final String FP_KEY_LEFT_NODE = "/sys/homebutton/key_left";
    public static final String FP_KEY_RIGHT_NODE = "/sys/homebutton/key_right";

    // Holds <preference_key> -> <proc_node> mapping
    public static final Map<String, String> sBooleanNodePreferenceMap = new HashMap<>();

    // Holds <preference_key> -> <default_values> mapping
    public static final Map<String, Object> sNodeDefaultMap = new HashMap<>();

    public static final String[] sButtonPrefKeys = {
        FP_HOME_KEY,
        FP_HOME_KEY_OFF,
        FP_KEYS,
        FP_KEY_HOLD,
        FP_KEY_RIGHT,
        FP_KEY_LEFT,
        FP_HAPTIC_KEY,
    };

    public static final String[] sButtonScreenOffPrefKeys = {
        FP_KEYS_OFF,
        FP_KEY_HOLD_OFF,
        FP_KEY_RIGHT_OFF,
        FP_KEY_LEFT_OFF,
        FP_HOME_KEY_OFF,
    };

    static {
        sBooleanNodePreferenceMap.put(FP_HOME_KEY, FP_HOME_NODE);
        sBooleanNodePreferenceMap.put(FP_HOME_KEY_OFF, FP_HOME_OFF_NODE);
        sBooleanNodePreferenceMap.put(FP_KEYS, FP_KEYS_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_HOLD, FP_KEY_HOLD_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_LEFT, FP_KEY_LEFT_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_RIGHT, FP_KEY_RIGHT_NODE);
        sBooleanNodePreferenceMap.put(FP_HAPTIC_KEY, FP_HAPTIC_NODE);
        sBooleanNodePreferenceMap.put(FP_KEYS_OFF, FP_KEYS_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_HOLD_OFF, FP_KEY_HOLD_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_LEFT_OFF, FP_KEY_LEFT_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_RIGHT_OFF, FP_KEY_RIGHT_NODE);
        sNodeDefaultMap.put(FP_HOME_KEY, false);
        sNodeDefaultMap.put(FP_HOME_KEY_OFF, false);
        sNodeDefaultMap.put(FP_KEYS, "0");
        sNodeDefaultMap.put(FP_KEY_HOLD, "0");
        sNodeDefaultMap.put(FP_KEY_LEFT, "0");
        sNodeDefaultMap.put(FP_KEY_RIGHT, "0");
        sNodeDefaultMap.put(FP_KEYS_OFF, "0");
        sNodeDefaultMap.put(FP_KEY_HOLD_OFF, "0");
        sNodeDefaultMap.put(FP_KEY_LEFT_OFF, "0");
        sNodeDefaultMap.put(FP_KEY_RIGHT_OFF, "0");
        sNodeDefaultMap.put(FP_HAPTIC_KEY, false);
    }

    public static boolean isPreferenceEnabled(Context context, String key) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        return preferences.getBoolean(key, (Boolean) sNodeDefaultMap.get(key));
    }

    public static String GetPreference(Context context, String key) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        return preferences.getString(key, (String) sNodeDefaultMap.get(key));
    }

    public static void writePreference(Context context, String pref) {

        String value = "1";
        Log.e(TAG, "Write Pref: " + pref);
        if (!pref.equals(FP_KEYS) && !pref.equals(FP_KEY_HOLD) && !pref.equals(FP_KEY_LEFT) && !pref.equals(FP_KEY_RIGHT) &&
            !pref.equals(FP_KEYS_OFF) && !pref.equals(FP_KEY_HOLD_OFF) && !pref.equals(FP_KEY_LEFT_OFF) && !pref.equals(FP_KEY_RIGHT_OFF))
            value = isPreferenceEnabled(context, pref) ? "1" : "0";
        else
            value = GetPreference(context, pref);

        String node = sBooleanNodePreferenceMap.get(pref);
            Log.e(TAG, "Write " + value + " to node " + node);

        if (!FileUtils.writeLine(node, value)) {
            Log.w(TAG, "Write " + value + " to node " + node +
                "failed while restoring saved preference values");
        }
    }
}

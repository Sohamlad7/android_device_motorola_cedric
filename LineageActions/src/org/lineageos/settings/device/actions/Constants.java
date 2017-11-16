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

import org.lineageos.settings.device.util.FileUtils;

public class Constants {

    public static final boolean DEBUG = false;

    private static final String TAG = "LineageActions";

    // FP gestures
    public static final int FP_TAP_SCANCODE = 616;
    public static final int FP_HOLD_SCANCODE = 617;
    public static final int FP_RIGHT_SCANCODE = 620;
    public static final int FP_LEFT_SCANCODE = 621;
    public static final int[] sSupportedFPGestures = new int[]{
            FP_TAP_SCANCODE,
            FP_HOLD_SCANCODE,
            FP_RIGHT_SCANCODE,
            FP_LEFT_SCANCODE
    };

    // FP actions
    public static final int ACTION_HOME = 100;
    public static final int ACTION_POWER = 101;
    public static final int ACTION_BACK = 102;
    public static final int ACTION_RECENTS = 103;
    public static final int ACTION_VOLUME_UP = 104;
    public static final int ACTION_VOLUME_DOWN = 105;
    public static final int ACTION_VOICE_ASSISTANT = 106;
    public static final int ACTION_PLAY_PAUSE = 107;
    public static final int ACTION_PREVIOUS_TRACK = 108;
    public static final int ACTION_NEXT_TRACK = 109;
    public static final int ACTION_FLASHLIGHT = 110;
    public static final int ACTION_CAMERA = 111;
    public static final int ACTION_SCREENSHOT = 112;
    public static final int ACTION_BROWSER = 116;
    public static final int ACTION_DIALER = 117;
    public static final int ACTION_EMAIL = 118;
    public static final int ACTION_MESSAGES = 119;
    public static final int ACTION_PIP = 120;
    public static final int ACTION_LAST_APP = 121;
    public static final int[] sFPSupportedActions = new int[]{
            ACTION_HOME,
            ACTION_POWER,
            ACTION_BACK,
            ACTION_RECENTS,
            ACTION_VOLUME_UP,
            ACTION_VOLUME_DOWN,
            ACTION_VOICE_ASSISTANT,
            ACTION_PLAY_PAUSE,
            ACTION_PREVIOUS_TRACK,
            ACTION_NEXT_TRACK,
            ACTION_FLASHLIGHT,
            ACTION_CAMERA,
            ACTION_SCREENSHOT,
            ACTION_PIP,
            ACTION_LAST_APP
    };
    public static final int[] sFPSupportedActionsScreenOff = new int[]{
            ACTION_POWER,
            ACTION_VOLUME_UP,
            ACTION_VOLUME_DOWN,
            ACTION_PLAY_PAUSE,
            ACTION_PREVIOUS_TRACK,
            ACTION_NEXT_TRACK,
            ACTION_FLASHLIGHT,
            ACTION_CAMERA
    };

    // Swap keys
    public static final String FP_HOME_KEY = "fp_home";
    public static final String FP_HOME_KEY_OFF = "fp_home_scr_off";

    // Swap nodes
    public static final String FP_HOME_NODE = "/sys/homebutton/enable";
    public static final String FP_HOME_OFF_NODE = "/sys/homebutton/enable_off";

    // Haptic node
    public static final String FP_HAPTIC_NODE = "/sys/homebutton/haptic";
    public static final String FP_HAPTIC_KEY = "fp_haptic";
    public static final String FP_HAPTIC_SCREENOFF_NODE = "/sys/homebutton/haptic_off";
    public static final String FP_HAPTIC_SCREENOFF_KEY = "fp_haptic_scr_off";

    // List of keys
    public static final String FP_KEYS = "fp_keys";
    public static final String FP_KEY_DBLTAP = "fp_key_dbltap";
    public static final String FP_KEY_HOLD = "fp_key_hold";
    public static final String FP_KEY_LEFT = "fp_key_left";
    public static final String FP_KEY_RIGHT = "fp_key_right";

    public static final String FP_KEYS_OFF = "fp_keys_off";
    public static final String FP_KEY_DBLTAP_OFF = "fp_key_dbltap_off";
    public static final String FP_KEY_HOLD_OFF = "fp_key_hold_off";
    public static final String FP_KEY_LEFT_OFF = "fp_key_left_off";
    public static final String FP_KEY_RIGHT_OFF = "fp_key_right_off";

    // Keys nodes
    public static final String FP_KEYS_NODE = "/sys/homebutton/key";
    public static final String FP_KEY_DBLTAP_NODE = "/sys/homebutton/key_dbltap";
    public static final String FP_KEY_HOLD_NODE = "/sys/homebutton/key_hold";
    public static final String FP_KEY_LEFT_NODE = "/sys/homebutton/key_left";
    public static final String FP_KEY_RIGHT_NODE = "/sys/homebutton/key_right";

    public static final String FP_KEYS_SCREENOFF_NODE = "/sys/homebutton/key_screenoff";
    public static final String FP_KEY_SCREENOFF_DBLTAP_NODE = "/sys/homebutton/key_screenoff_dbltap";
    public static final String FP_KEY_SCREENOFF_HOLD_NODE = "/sys/homebutton/key_screenoff_hold";
    public static final String FP_KEY_SCREENOFF_LEFT_NODE = "/sys/homebutton/key_screenoff_left";
    public static final String FP_KEY_SCREENOFF_RIGHT_NODE = "/sys/homebutton/key_screenoff_right";

    // Screen off gestures
    public static final int GESTURE_SWIPE_RIGHT_SCANCODE = 622;
    public static final int GESTURE_SWIPE_LEFT_SCANCODE = 623;
    public static final int GESTURE_SWIPE_DOWN_SCANCODE = 624;
    public static final int GESTURE_SWIPE_UP_SCANCODE = 625;
    public static final int GESTURE_DOUBLE_TAP_SCANCODE = 626;
    public static final int[] sSupportedScreenOffGestures = new int[]{
            GESTURE_SWIPE_RIGHT_SCANCODE,
            GESTURE_SWIPE_LEFT_SCANCODE,
            GESTURE_SWIPE_DOWN_SCANCODE,
            GESTURE_SWIPE_UP_SCANCODE,
            GESTURE_DOUBLE_TAP_SCANCODE
    };
    public static final int[] sScreenOffSupportedActions = new int[]{
            ACTION_POWER,
            ACTION_PLAY_PAUSE,
            ACTION_PREVIOUS_TRACK,
            ACTION_NEXT_TRACK,
            ACTION_FLASHLIGHT,
            ACTION_CAMERA,
            ACTION_BROWSER,
            ACTION_DIALER,
            ACTION_EMAIL,
            ACTION_MESSAGES
    };

    // List of screen off gestures keys
    public static final String GESTURE_SWIPE_RIGHT = "screen_off_gestures_swipe_right";
    public static final String GESTURE_SWIPE_LEFT = "screen_off_gestures_swipe_left";
    public static final String GESTURE_SWIPE_DOWN = "screen_off_gestures_swipe_down";
    public static final String GESTURE_SWIPE_UP = "screen_off_gestures_swipe_up";

    // Screen off gestures nodes
    public static final String GESTURE_SWIPE_RIGHT_NODE = "/sys/android_touch/gesture_swipe_right";
    public static final String GESTURE_SWIPE_LEFT_NODE = "/sys/android_touch/gesture_swipe_left";
    public static final String GESTURE_SWIPE_DOWN_NODE = "/sys/android_touch/gesture_swipe_down";
    public static final String GESTURE_SWIPE_UP_NODE = "/sys/android_touch/gesture_swipe_up";
    public static final String GESTURE_DOUBLE_TAP_NODE = "/sys/android_touch/doubletap2wake";
    
    // Screen off gestures haptic
    public static final String KEY_GESTURE_ENABLE_HAPTIC_FEEDBACK = "screen_off_gesture_haptic_feedback";

    // Holds <preference_key> -> <proc_node> mapping
    public static final Map<String, String> sBooleanNodePreferenceMap = new HashMap<>();

    // Holds <preference_key> -> <default_values> mapping
    public static final Map<String, Object> sNodeDefaultMap = new HashMap<>();

    public static final String[] sPrefKeys = {
        FP_HOME_KEY,
        FP_HOME_KEY_OFF,
        FP_HAPTIC_KEY,
        FP_HAPTIC_SCREENOFF_KEY,
        FP_KEYS,
        FP_KEY_DBLTAP,
        FP_KEY_HOLD,
        FP_KEY_RIGHT,
        FP_KEY_LEFT,
        FP_KEYS_OFF,
        FP_KEY_DBLTAP_OFF,
        FP_KEY_HOLD_OFF,
        FP_KEY_RIGHT_OFF,
        FP_KEY_LEFT_OFF,
        FP_HOME_KEY_OFF,
        GESTURE_SWIPE_RIGHT,
        GESTURE_SWIPE_LEFT,
        GESTURE_SWIPE_DOWN,
        GESTURE_SWIPE_UP
    };

    static {
        sBooleanNodePreferenceMap.put(FP_HOME_KEY, FP_HOME_NODE);
        sBooleanNodePreferenceMap.put(FP_HOME_KEY_OFF, FP_HOME_OFF_NODE);
        sBooleanNodePreferenceMap.put(FP_HAPTIC_KEY, FP_HAPTIC_NODE);
        sBooleanNodePreferenceMap.put(FP_HAPTIC_SCREENOFF_KEY, FP_HAPTIC_SCREENOFF_NODE);
        sBooleanNodePreferenceMap.put(FP_KEYS, FP_KEYS_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_DBLTAP, FP_KEY_DBLTAP_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_HOLD, FP_KEY_HOLD_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_LEFT, FP_KEY_LEFT_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_RIGHT, FP_KEY_RIGHT_NODE);
        sBooleanNodePreferenceMap.put(FP_KEYS_OFF, FP_KEYS_SCREENOFF_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_DBLTAP_OFF, FP_KEY_SCREENOFF_DBLTAP_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_HOLD_OFF, FP_KEY_SCREENOFF_HOLD_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_LEFT_OFF, FP_KEY_SCREENOFF_LEFT_NODE);
        sBooleanNodePreferenceMap.put(FP_KEY_RIGHT_OFF, FP_KEY_SCREENOFF_RIGHT_NODE);
        sBooleanNodePreferenceMap.put(GESTURE_SWIPE_RIGHT, GESTURE_SWIPE_RIGHT_NODE);
        sBooleanNodePreferenceMap.put(GESTURE_SWIPE_LEFT, GESTURE_SWIPE_LEFT_NODE);
        sBooleanNodePreferenceMap.put(GESTURE_SWIPE_DOWN, GESTURE_SWIPE_DOWN_NODE);
        sBooleanNodePreferenceMap.put(GESTURE_SWIPE_UP, GESTURE_SWIPE_UP_NODE);
        sNodeDefaultMap.put(FP_HOME_KEY, false);
        sNodeDefaultMap.put(FP_HOME_KEY_OFF, false);
        sNodeDefaultMap.put(FP_HAPTIC_KEY, false);
        sNodeDefaultMap.put(FP_HAPTIC_SCREENOFF_KEY, false);
        sNodeDefaultMap.put(FP_KEYS, "0");
        sNodeDefaultMap.put(FP_KEY_DBLTAP, "0");
        sNodeDefaultMap.put(FP_KEY_HOLD, "0");
        sNodeDefaultMap.put(FP_KEY_LEFT, "0");
        sNodeDefaultMap.put(FP_KEY_RIGHT, "0");
        sNodeDefaultMap.put(FP_KEYS_OFF, "0");
        sNodeDefaultMap.put(FP_KEY_DBLTAP_OFF, "0");
        sNodeDefaultMap.put(FP_KEY_HOLD_OFF, "0");
        sNodeDefaultMap.put(FP_KEY_LEFT_OFF, "0");
        sNodeDefaultMap.put(FP_KEY_RIGHT_OFF, "0");
        sNodeDefaultMap.put(GESTURE_SWIPE_RIGHT, "0");
        sNodeDefaultMap.put(GESTURE_SWIPE_LEFT, "0");
        sNodeDefaultMap.put(GESTURE_SWIPE_DOWN, "0");
        sNodeDefaultMap.put(GESTURE_SWIPE_UP, "0");
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
        if (!pref.equals(FP_KEYS) && !pref.equals(FP_KEY_DBLTAP) && !pref.equals(FP_KEY_HOLD) && !pref.equals(FP_KEY_LEFT) && !pref.equals(FP_KEY_RIGHT) &&
            !pref.equals(FP_KEYS_OFF) && !pref.equals(FP_KEY_DBLTAP_OFF) && !pref.equals(FP_KEY_HOLD_OFF) && !pref.equals(FP_KEY_LEFT_OFF) && !pref.equals(FP_KEY_RIGHT_OFF) && !pref.equals(GESTURE_SWIPE_RIGHT) && !pref.equals(GESTURE_SWIPE_LEFT) && !pref.equals(GESTURE_SWIPE_DOWN) && !pref.equals(GESTURE_SWIPE_UP))
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

/*
 * Copyright (c) 2016 The CyanogenMod Project
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

package org.lineageos.settings.device;

import android.os.Bundle;
import android.provider.Settings;
import android.support.v7.preference.Preference;
import android.support.v14.preference.SwitchPreference;
import android.support.v14.preference.PreferenceFragment;
import android.view.MenuItem;

import static android.provider.Settings.Secure.DOUBLE_TAP_TO_WAKE;
import static org.lineageos.settings.device.actions.Constants.KEY_GESTURE_ENABLE_HAPTIC_FEEDBACK;

public class ScreenOffGestureSettingsFragment extends PreferenceFragment {

    private SwitchPreference mTapToWake;
    private SwitchPreference mHapticFeedback;

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        getActivity().getActionBar().setDisplayHomeAsUpEnabled(true);
    }

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.screen_off_gesture_panel);
        mTapToWake = (SwitchPreference) findPreference("tap_to_wake");
        mTapToWake.setChecked(Settings.Secure.getInt(getActivity().getContentResolver(), DOUBLE_TAP_TO_WAKE, 0) == 1);
        mTapToWake.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object objValue) {
                boolean value = (Boolean) objValue;
                Settings.Secure.putInt(getActivity().getContentResolver(), DOUBLE_TAP_TO_WAKE, value ? 1 : 0);
                return true;
            }
        });

        mHapticFeedback = (SwitchPreference) findPreference("haptic_feedback");
        mHapticFeedback.setChecked(Settings.System.getInt(getActivity().getContentResolver(), KEY_GESTURE_ENABLE_HAPTIC_FEEDBACK, 1) == 1);
        mHapticFeedback.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object objValue) {
                boolean value = (Boolean) objValue;
                Settings.System.putInt(getActivity().getContentResolver(), KEY_GESTURE_ENABLE_HAPTIC_FEEDBACK, value ? 1 : 0);
                return true;
            }
        });
    }

}

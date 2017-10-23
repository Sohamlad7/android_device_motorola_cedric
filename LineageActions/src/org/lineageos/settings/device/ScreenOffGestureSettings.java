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
import android.preference.Preference;
import android.preference.SwitchPreference;
import android.preference.PreferenceActivity;
import android.view.MenuItem;

import static android.provider.Settings.Secure.DOUBLE_TAP_TO_WAKE;
import static org.lineageos.settings.device.actions.Constants.KEY_GESTURE_ENABLE_HAPTIC_FEEDBACK;

public class ScreenOffGestureSettings extends PreferenceActivity {

    private SwitchPreference mTapToWake;
    private SwitchPreference mHapticFeedback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.screen_off_gesture_panel);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        mTapToWake = (SwitchPreference) findPreference("tap_to_wake");
        mTapToWake.setChecked(Settings.Secure.getInt(getContentResolver(), DOUBLE_TAP_TO_WAKE, 0) == 1);
        mTapToWake.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object objValue) {
                boolean value = (Boolean) objValue;
                Settings.Secure.putInt(getContentResolver(), DOUBLE_TAP_TO_WAKE, value ? 1 : 0);
                return true;
            }
        });

        mHapticFeedback = (SwitchPreference) findPreference("haptic_feedback");
        mHapticFeedback.setChecked(Settings.System.getInt(getContentResolver(), KEY_GESTURE_ENABLE_HAPTIC_FEEDBACK, 1) == 1);
        mHapticFeedback.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object objValue) {
                boolean value = (Boolean) objValue;
                Settings.System.putInt(getContentResolver(), KEY_GESTURE_ENABLE_HAPTIC_FEEDBACK, value ? 1 : 0);
                return true;
            }
        });
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

}

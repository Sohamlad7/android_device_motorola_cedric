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
import android.content.Context;
import android.preference.PreferenceCategory;
import android.preference.SwitchPreference;
import android.hardware.fingerprint.FingerprintManager;
import android.preference.PreferenceActivity;
import android.view.MenuItem;

public class FPGestureSettings extends PreferenceActivity {

    private SwitchPreference mFPScreenOffGesture;
    private PreferenceCategory mFPScreenOffCategory;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.fp_gesture_panel);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        mFPScreenOffGesture = (SwitchPreference) findPreference("fp_home_scr_off");
        mFPScreenOffCategory = (PreferenceCategory) findPreference("fp_keys_scr_off");
        boolean hasEnrolledFingerprints = hasEnrolledFingerprints();
        mFPScreenOffGesture.setEnabled(!hasEnrolledFingerprints);
        mFPScreenOffCategory.setEnabled(!hasEnrolledFingerprints);
    }

    private boolean hasEnrolledFingerprints(){
        FingerprintManager fingerprintManager = (FingerprintManager) getSystemService(Context.FINGERPRINT_SERVICE);
        return fingerprintManager.hasEnrolledFingerprints();
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

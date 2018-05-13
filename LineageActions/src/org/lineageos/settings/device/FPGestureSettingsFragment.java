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

import android.app.ActionBar;
import android.os.Bundle;
import android.content.Context;
import android.content.SharedPreferences.Editor;
import android.preference.PreferenceManager;
import android.support.v7.preference.PreferenceCategory;
import android.support.v14.preference.SwitchPreference;
import android.hardware.fingerprint.FingerprintManager;
import android.support.v14.preference.PreferenceFragment;
import android.view.MenuItem;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import org.lineageos.settings.device.actions.Constants;

import static org.lineageos.settings.device.actions.Constants.FP_HOME_KEY;
import static org.lineageos.settings.device.actions.Constants.FP_HOME_KEY_OFF;

public class FPGestureSettingsFragment extends PreferenceFragment {

    private SwitchPreference mFPScreenOffGesture;
    private PreferenceCategory mFPScreenOffCategory;
    private PreferenceCategory mFPScreenOnCategory;

    private TextView mSwitchBarText;
    private Switch mFPGestureSwitch;

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        ActionBar actionbar = getActivity().getActionBar();
        actionbar.setDisplayHomeAsUpEnabled(true);
        actionbar.setTitle(R.string.fingerprint_gestures_title);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        final View view = LayoutInflater.from(getContext()).inflate(R.layout.fp_gesture, container, false);
        ((ViewGroup) view).addView(super.onCreateView(inflater, container, savedInstanceState));
        return view;
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        View switchBar = view.findViewById(R.id.switch_bar);
        mFPGestureSwitch = (Switch) switchBar.findViewById(android.R.id.switch_widget);
        mFPGestureSwitch.setChecked(isFPGestureEnabled());
        mFPGestureSwitch.setOnCheckedChangeListener(mFPGesturePrefListener);

        switchBar.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mFPGestureSwitch.toggle();
            }
        });

        mSwitchBarText = switchBar.findViewById(R.id.switch_text);
        mSwitchBarText.setText(isFPGestureEnabled() ? R.string.switch_bar_on :
                R.string.switch_bar_off);
    }
    
    private void updatePrefs(boolean enabled){
        Editor prefEditor = PreferenceManager.getDefaultSharedPreferences(getActivity()).edit();
        prefEditor.putBoolean(FP_HOME_KEY, enabled);
        prefEditor.apply(); 
        mFPScreenOnCategory.setEnabled(enabled);
        mFPScreenOffGesture.setEnabled(enabled);
        mFPScreenOffCategory.setEnabled(enabled);
        if(enabled){
            boolean hasEnrolledFingerprints = hasEnrolledFingerprints();
            mFPScreenOffGesture.setEnabled(!hasEnrolledFingerprints);
            mFPScreenOffCategory.setEnabled(!hasEnrolledFingerprints);
        }
    }
    
    private boolean isFPGestureEnabled(){
        return Constants.isPreferenceEnabled(getActivity(), FP_HOME_KEY);
    }

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.fp_gesture_panel);
        mFPScreenOffGesture = (SwitchPreference) findPreference(FP_HOME_KEY_OFF);
        mFPScreenOffCategory = (PreferenceCategory) findPreference("fp_keys_scr_off");
        mFPScreenOnCategory = (PreferenceCategory) findPreference("fp_keys_scr_on");
        updatePrefs(isFPGestureEnabled());
    }

    private CompoundButton.OnCheckedChangeListener mFPGesturePrefListener =
        new CompoundButton.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton compoundButton, boolean enable) {
            updatePrefs(enable);
            mSwitchBarText.setText(enable ? R.string.switch_bar_on : R.string.switch_bar_off);
        }
    };

    private boolean hasEnrolledFingerprints(){
        FingerprintManager fingerprintManager = (FingerprintManager) getActivity().getSystemService(Context.FINGERPRINT_SERVICE);
        return fingerprintManager.hasEnrolledFingerprints();
    }

}

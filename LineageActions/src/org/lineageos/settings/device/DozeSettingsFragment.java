/*
 * Copyright (c) 2017 The LineageOS Project
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
import android.support.v14.preference.PreferenceFragment;
import android.provider.Settings;
import android.support.v7.preference.Preference;
import android.support.v14.preference.SwitchPreference;
import android.view.MenuItem;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

public class DozeSettingsFragment extends PreferenceFragment {
    
    private SwitchPreference mHandwavePreference;
    private SwitchPreference mPickupPreference;

    private TextView mSwitchBarText;
    private Switch mAmbientDisplaySwitch;

    private String KEY_GESTURE_HAND_WAVE = "gesture_hand_wave";
    private String KEY_GESTURE_PICK_UP = "gesture_pick_up";
    
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        ActionBar actionbar = getActivity().getActionBar();
        actionbar.setDisplayHomeAsUpEnabled(true);
        actionbar.setTitle(R.string.ambient_display_title);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        final View view = LayoutInflater.from(getContext()).inflate(R.layout.doze, container, false);
        ((ViewGroup) view).addView(super.onCreateView(inflater, container, savedInstanceState));
        return view;
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        View switchBar = view.findViewById(R.id.switch_bar);
        mAmbientDisplaySwitch = (Switch) switchBar.findViewById(android.R.id.switch_widget);
        mAmbientDisplaySwitch.setChecked(LineageActionsSettings.isDozeEnabled(getActivity().getContentResolver()));
        mAmbientDisplaySwitch.setOnCheckedChangeListener(mAmbientDisplayPrefListener);

        switchBar.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mAmbientDisplaySwitch.toggle();
            }
        });

        mSwitchBarText = switchBar.findViewById(R.id.switch_text);
        mSwitchBarText.setText(LineageActionsSettings.isDozeEnabled(getActivity().getContentResolver()) ? R.string.switch_bar_on :
                R.string.switch_bar_off);
    }

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.doze_panel);
        boolean dozeEnabled = LineageActionsSettings.isDozeEnabled(getActivity().getContentResolver());
        mHandwavePreference = (SwitchPreference) findPreference(KEY_GESTURE_HAND_WAVE);
        mPickupPreference = (SwitchPreference) findPreference(KEY_GESTURE_PICK_UP);
        updatePrefs(dozeEnabled);
    }

    private CompoundButton.OnCheckedChangeListener mAmbientDisplayPrefListener =
        new CompoundButton.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton compoundButton, boolean enable) {
            if (enableDoze(enable)) {
                updatePrefs(enable);
                mSwitchBarText.setText(enable ? R.string.switch_bar_on : R.string.switch_bar_off);
            }
        }
    };
    
    private void updatePrefs(boolean enabled){
        mHandwavePreference.setEnabled(enabled);
        mPickupPreference.setEnabled(enabled);
    }

    private boolean enableDoze(boolean enable) {
        return Settings.Secure.putInt(getActivity().getContentResolver(),
                Settings.Secure.DOZE_ENABLED, enable ? 1 : 0);
    }
}

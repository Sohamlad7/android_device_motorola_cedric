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

package com.cyanogenmod.settings.navibar;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.provider.Settings;
import android.view.Menu;
import android.view.MenuItem;

public class NaviBarSettings extends PreferenceActivity implements OnPreferenceChangeListener {

    private Context mContext;
    private SharedPreferences mPreferences;

    private SwitchPreference mNavigationBarPreference;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.navi_bar_settings);
        mContext = getApplicationContext();
        boolean navibarEnabled = Utils.isNaviBarEnabled(mContext);

        mPreferences = mContext.getSharedPreferences("navibar_settings", Activity.MODE_PRIVATE);

        mNavigationBarPreference =
            (SwitchPreference) findPreference(Utils.NAVI_BAR_KEY);
        mNavigationBarPreference.setChecked(navibarEnabled);
        mNavigationBarPreference.setOnPreferenceChangeListener(this);

        final ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return false;
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        final String key = preference.getKey();
        final boolean value = (Boolean) newValue;
        if (Utils.NAVI_BAR_KEY.equals(key)) {
            mNavigationBarPreference.setChecked(value);
            Utils.enableNaviBar(value, mContext);
            return true;
        }
        return false;
    }
}

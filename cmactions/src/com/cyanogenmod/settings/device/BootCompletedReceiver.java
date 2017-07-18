/*
 * Copyright (c) 2015 The CyanogenMod Project
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

package com.cyanogenmod.settings.device;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.util.Log;

import cyanogenmod.providers.CMSettings;

import com.cyanogenmod.settings.device.ServiceWrapper.LocalBinder;

public class BootCompletedReceiver extends BroadcastReceiver {
    static final String TAG = "CMActions";
    final String NAVBAR_SHOWN = "navbar_shown";
    private ServiceWrapper mServiceWrapper;

    @Override
    public void onReceive(final Context context, Intent intent) {
        Log.i(TAG, "Booting");

        // Restore nodes to saved preference values
        for (String pref : Constants.sButtonPrefKeys) {
             Constants.writePreference(context, pref);
        }

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);

        if (!preferences.getBoolean(NAVBAR_SHOWN, false)) {
            enableNaviBar(true, context);
            preferences.edit().putBoolean(NAVBAR_SHOWN, true).commit(); 
        }

        context.startService(new Intent(context, ServiceWrapper.class));
    }

    protected static void enableNaviBar(boolean enable, Context context) {
        CMSettings.Global.putInt(context.getContentResolver(),
                CMSettings.Global.DEV_FORCE_SHOW_NAVBAR, enable ? 1 : 0);
    }

    private ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            LocalBinder binder = (LocalBinder) service;
            mServiceWrapper = binder.getService();
            mServiceWrapper.start();
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            mServiceWrapper = null;
        }
    };
}

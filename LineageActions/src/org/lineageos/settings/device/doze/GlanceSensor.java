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

package org.lineageos.settings.device.doze;

import android.hardware.Sensor;
import android.hardware.TriggerEvent;
import android.hardware.TriggerEventListener;
import android.util.Log;

import org.lineageos.settings.device.LineageActionsSettings;
import org.lineageos.settings.device.SensorAction;
import org.lineageos.settings.device.SensorHelper;

public class GlanceSensor implements ScreenStateNotifier {
    private static final String TAG = "LineageActions-GlanceSensor";

    private final LineageActionsSettings mLineageActionsSettings;
    private final SensorHelper mSensorHelper;
    private final SensorAction mSensorAction;
    private final Sensor mSensor;

    private boolean mEnabled;

    public GlanceSensor(LineageActionsSettings lineageActionsSettings, SensorHelper sensorHelper,
                SensorAction action) {
        mLineageActionsSettings = lineageActionsSettings;
        mSensorHelper = sensorHelper;
        mSensorAction = action;

        mSensor = sensorHelper.getGlanceSensor();
    }

    @Override
    public void screenTurnedOn() {
        if (mEnabled) {
            Log.d(TAG, "Disabling");
            mSensorHelper.cancelTriggerSensor(mSensor, mGlanceListener);
            mEnabled = false;
        }
    }

    @Override
    public void screenTurnedOff() {
        if (mLineageActionsSettings.isPickUpEnabled() && !mEnabled) {
            Log.d(TAG, "Enabling");
            mSensorHelper.requestTriggerSensor(mSensor, mGlanceListener);
            mEnabled = true;
        }
    }

    private TriggerEventListener mGlanceListener = new TriggerEventListener() {
        @Override
        public void onTrigger(TriggerEvent event) {
            Log.d(TAG, "triggered");
            mSensorAction.action();
            mSensorHelper.requestTriggerSensor(mSensor, mGlanceListener);
        }
    };
}

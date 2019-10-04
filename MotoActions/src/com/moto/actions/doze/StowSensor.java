/*
 * Copyright (c) 2015 The CyanogenMod Project
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

package com.moto.actions.doze;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.util.Log;

import java.lang.System;

import com.moto.actions.MotoActionsSettings;
import com.moto.actions.SensorAction;
import com.moto.actions.SensorHelper;

public class StowSensor implements ScreenStateNotifier, SensorEventListener {
    private static final String TAG = "MotoActions-StowSensor";
    private static final int IN_POCKET_MIN_TIME = 5000;

    private final MotoActionsSettings mMotoActionsSettings;
    private final SensorHelper mSensorHelper;
    private final SensorAction mSensorAction;
    private final Sensor mSensor;

    private boolean mEnabled;
    private boolean mLastStowed;
    private long isStowedTime;

    public StowSensor(MotoActionsSettings motoActionsSettings, SensorHelper sensorHelper,
                SensorAction action) {
        mMotoActionsSettings = motoActionsSettings;
        mSensorHelper = sensorHelper;
        mSensorAction = action;

        mSensor = sensorHelper.getStowSensor();
    }

    @Override
    public void screenTurnedOn() {
        if (mEnabled) {
            Log.d(TAG, "Disabling");
            mSensorHelper.unregisterListener(this);
            mEnabled = false;
        }
    }

    @Override
    public void screenTurnedOff() {
        if (!mMotoActionsSettings.isIrWakeupEnabled() &&
            mMotoActionsSettings.isPickUpEnabled() && !mEnabled) {
            Log.d(TAG, "Enabling");
            mSensorHelper.registerListener(mSensor, this);
            mEnabled = true;
        }
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        boolean thisStowed = (event.values[0] != 0);
        if(thisStowed){
            isStowedTime = System.currentTimeMillis();
        } else if (mLastStowed && !thisStowed) {
            long inPocketTime = System.currentTimeMillis() - isStowedTime;
            if(inPocketTime >= IN_POCKET_MIN_TIME){
                Log.d(TAG, "Triggered after " + inPocketTime / 1000 + " seconds");
                mSensorAction.action();
            }
        }
        mLastStowed = thisStowed;
        Log.d(TAG, "event: " + thisStowed);
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }
}

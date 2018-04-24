/*
 * Copyright (C) 2015 The CyanogenMod Project
 * Copyright (C) 2018 The LineageOS Project
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

package org.lineageos.hardware;

import android.os.IHwBinder.DeathRecipient;
import android.os.RemoteException;
import android.util.Log;
import android.util.Range;
import com.android.internal.annotations.GuardedBy;

import lineageos.hardware.DisplayMode;
import lineageos.hardware.HSIC;
import vendor.lineage.livedisplay.V1_0.IColor;

/**
 * This class loads an implementation of the LiveDisplay native interface.
 */
public class LiveDisplayVendorImpl {

    private static final String TAG = "LiveDisplayVendorImpl";

    @GuardedBy("this")
    private IColor mDaemon;
    private int mFeatures;

    private LiveDisplayVendorImpl() {}

    private static class LiveDisplayVendorImplHolder {
        private static final LiveDisplayVendorImpl instance = new LiveDisplayVendorImpl();
    }

    public static LiveDisplayVendorImpl getInstance() {
        return LiveDisplayVendorImplHolder.instance;
    }

    private synchronized IColor getColorService() {
        if (mDaemon == null) {
            Log.v(TAG, "mDaemon was null, reconnect to LiveDisplay IColor");
            try {
                mDaemon = IColor.getService();
            } catch (java.util.NoSuchElementException e) {
                // Service doesn't exist or cannot be opened. Logged below.
            } catch (RemoteException e) {
                Log.e(TAG, "Failed to get LiveDisplay IColor interface", e);
            }
            if (mDaemon == null) {
                Log.w(TAG, "LiveDisplay IColor HIDL not available");
                return null;
            }

            mDaemon.asBinder().linkToDeath(new DeathRecipient() {
                @Override
                public void serviceDied(long cookie) {
                    Log.e(TAG, "LiveDisplay IColor HAL died");
                    reset();
                }
            }, 0);
        }
        return mDaemon;
    }

    private void reset() {
        mFeatures = 0;
        synchronized (this) {
            mDaemon = null;
        }
    }

    public boolean hasNativeFeature(int feature) {
        if (mFeatures == 0) {
            IColor daemon = getColorService();
            if (daemon == null) {
                Log.e(TAG, "hasNativeFeature: no LiveDisplay IColor HAL!");
                return false;
            }
            try {
                mFeatures = daemon.getSupportedFeatures();
                Log.i(TAG, "Using LiveDisplay IColor backend (features: " + mFeatures + ")");
            } catch (RemoteException e) {
                Log.e(TAG, "hasNativeFeature failed", e);
                reset();
                return false;
            }
        }
        Log.d(TAG, "hasNativeFeature: mFeatures=" + Integer.toString(mFeatures));
        return (mFeatures & feature) != 0;
    }

    public DisplayMode[] getDisplayModes() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getDisplayModes: no LiveDisplay IColor HAL!");
            return null;
        }
        try {
            return Utils.HIDLModeListToArray(daemon.getDisplayModes());
        } catch (RemoteException e) {
            Log.e(TAG, "getDisplayModes failed", e);
            reset();
        }
        return null;
    }

    public DisplayMode getCurrentDisplayMode() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getCurrentDisplayMode: no LiveDisplay IColor HAL!");
            return null;
        }
        try {
            DisplayMode mode = Utils.fromHIDLMode(daemon.getCurrentDisplayMode());
            // mode.id is -1 means it's invalid.
            return mode.id == -1 ? null : mode;
        } catch (RemoteException e) {
            Log.e(TAG, "getDisplayModes failed", e);
            reset();
        }
        return null;
    }

    public DisplayMode getDefaultDisplayMode() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getDefaultDisplayMode: no LiveDisplay IColor HAL!");
            return null;
        }
        try {
            DisplayMode mode = Utils.fromHIDLMode(daemon.getDefaultDisplayMode());
            // mode.id is -1 means it's invalid.
            return mode.id == -1 ? null : mode;
        } catch (RemoteException e) {
            Log.e(TAG, "getDefaultDisplayMode failed", e);
            reset();
        }
        return null;
    }

    public boolean setDisplayMode(DisplayMode mode, boolean makeDefault) {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "setDisplayMode: no LiveDisplay IColor HAL!");
            return false;
        }
        try {
            return daemon.setDisplayMode(mode.id, makeDefault);
        } catch (RemoteException e) {
            Log.e(TAG, "setDisplayMode failed", e);
            reset();
        }
        return false;
    }

    public boolean setAdaptiveBacklightEnabled(boolean enabled) {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "setAdaptiveBacklightEnabled: no LiveDisplay IColor HAL!");
            return false;
        }
        try {
            return daemon.setAdaptiveBacklightEnabled(enabled);
        } catch (RemoteException e) {
            Log.e(TAG, "setAdaptiveBacklightEnabled failed", e);
            reset();
        }
        return false;
    }

    public boolean isAdaptiveBacklightEnabled() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "isAdaptiveBacklightEnabled: no LiveDisplay IColor HAL!");
            return false;
        }
        try {
            return daemon.isAdaptiveBacklightEnabled();
        } catch (RemoteException e) {
            Log.e(TAG, "isAdaptiveBacklightEnabled failed", e);
            reset();
        }
        return false;
    }

    public boolean setOutdoorModeEnabled(boolean enabled) {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "setOutdoorModeEnabled: no LiveDisplay IColor HAL!");
            return false;
        }
        try {
            return daemon.setOutdoorModeEnabled(enabled);
        } catch (RemoteException e) {
            Log.e(TAG, "setOutdoorModeEnabled failed", e);
            reset();
        }
        return false;
    }

    public boolean isOutdoorModeEnabled() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "isOutdoorModeEnabled: no LiveDisplay IColor HAL!");
            return false;
        }
        try {
            return daemon.isOutdoorModeEnabled();
        } catch (RemoteException e) {
            Log.e(TAG, "isOutdoorModeEnabled failed", e);
            reset();
        }
        return false;
    }

    public Range<Integer> getColorBalanceRange() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getColorBalanceRange: no LiveDisplay IColor HAL!");
            return null;
        }
        try {
            return Utils.fromHIDLRange(daemon.getColorBalanceRange());
        } catch (RemoteException e) {
            Log.e(TAG, "getColorBalanceRange failed", e);
            reset();
        }
        return null;
    }

    public int getColorBalance() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getColorBalance: no LiveDisplay IColor HAL!");
            return 0;
        }
        try {
            return daemon.getColorBalance();
        } catch (RemoteException e) {
            Log.e(TAG, "getColorBalance failed", e);
            reset();
        }
        return 0;
    }

    public boolean setColorBalance(int value) {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "setColorBalance: no LiveDisplay IColor HAL!");
            return false;
        }
        try {
            return daemon.setColorBalance(value);
        } catch (RemoteException e) {
            Log.e(TAG, "setColorBalance failed", e);
            reset();
        }
        return false;
    }

    public boolean setPictureAdjustment(final HSIC hsic) {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "setPictureAdjustment: no LiveDisplay IColor HAL!");
            return false;
        }
        try {
            return daemon.setPictureAdjustment(Utils.toHIDLHSIC(hsic));
        } catch (RemoteException e) {
            Log.e(TAG, "setPictureAdjustment failed", e);
            reset();
        }
        return false;
    }

    public HSIC getPictureAdjustment() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getPictureAdjustment: no LiveDisplay IColor HAL!");
            return null;
        }
        try {
            return Utils.fromHIDLHSIC(daemon.getPictureAdjustment());
        } catch (RemoteException e) {
            Log.e(TAG, "getPictureAdjustment failed", e);
            reset();
        }
        return null;
    }

    public HSIC getDefaultPictureAdjustment() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getDefaultPictureAdjustment: no LiveDisplay IColor HAL!");
            return null;
        }
        try {
            return Utils.fromHIDLHSIC(daemon.getDefaultPictureAdjustment());
        } catch (RemoteException e) {
            Log.e(TAG, "getDefaultPictureAdjustment failed", e);
            reset();
        }
        return null;
    }

    public Range<Float> getHueRange() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getHueRange: no LiveDisplay IColor HAL!");
            return null;
        }
        try {
            return Utils.fromHIDLIntRange(daemon.getHueRange());
        } catch (RemoteException e) {
            Log.e(TAG, "getHueRange failed", e);
            reset();
        }
        return null;
    }

    public Range<Float> getSaturationRange() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getSaturationRange: no LiveDisplay IColor HAL!");
            return null;
        }
        try {
            return Utils.fromHIDLRange(daemon.getSaturationRange());
        } catch (RemoteException e) {
            Log.e(TAG, "getSaturationRange failed", e);
            reset();
        }
        return null;
    }

    public Range<Float> getIntensityRange() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getIntensityRange: no LiveDisplay IColor HAL!");
            return null;
        }
        try {
            return Utils.fromHIDLRange(daemon.getIntensityRange());
        } catch (RemoteException e) {
            Log.e(TAG, "getIntensityRange failed", e);
            reset();
        }
        return null;
    }

    public Range<Float> getContrastRange() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getContrastRange: no LiveDisplay IColor HAL!");
            return null;
        }
        try {
            return Utils.fromHIDLRange(daemon.getContrastRange());
        } catch (RemoteException e) {
            Log.e(TAG, "getContrastRange failed", e);
            reset();
        }
        return null;
    }

    public Range<Float> getSaturationThresholdRange() {
        IColor daemon = getColorService();
        if (daemon == null) {
            Log.e(TAG, "getSaturationThresholdRange: no LiveDisplay IColor HAL!");
            return null;
        }
        try {
            return Utils.fromHIDLRange(daemon.getSaturationThresholdRange());
        } catch (RemoteException e) {
            Log.e(TAG, "getSaturationThresholdRange failed", e);
            reset();
        }
        return null;
    }
}

/*
 * Copyright (C) 2016 The CyanogenMod Project
 * Copyright (C) 2017 The LineageOS Project
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

import android.app.ActivityManager;
import android.app.ActivityManagerNative;
import android.app.ISearchManager;
import android.app.KeyguardManager;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.input.InputManager;
import android.media.AudioAttributes;
import android.media.session.MediaSessionLegacyHelper;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.UserHandle;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.provider.MediaStore;
import android.provider.Settings;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.ViewConfiguration;
import android.widget.Toast;

import com.android.internal.os.DeviceKeyHandler;
import com.android.internal.util.ArrayUtils;

import org.lineageos.internal.util.FileUtils;

import java.util.List;

import lineageos.providers.LineageSettings;

import static org.lineageos.settings.device.actions.Constants.*;

public class KeyHandler implements DeviceKeyHandler {

    private static final String TAG = KeyHandler.class.getSimpleName();

    private static final int GESTURE_REQUEST = 1;
    private static final int FP_ACTION_REQUEST = 2;

    private static final String ACTION_DISMISS_KEYGUARD =
            "com.android.keyguard.action.DISMISS_KEYGUARD_SECURELY";

    private static final String GESTURE_WAKEUP_REASON = "keyhandler-gesture-wakeup";
    private static final int GESTURE_WAKELOCK_DURATION = 3000;
    private static final AudioAttributes VIBRATION_ATTRIBUTES = new AudioAttributes.Builder()
            .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
            .setUsage(AudioAttributes.USAGE_ASSISTANCE_SONIFICATION)
            .build();
    private static final long[] LONG_PRESS_VIBRATION_PATTERN = new long[] {0, 25, 10};
    private final Context mContext;
    private final PowerManager mPowerManager;
    WakeLock mProximityWakeLock;
    WakeLock mGestureWakeLock;
    private KeyguardManager mKeyguardManager;
    private ScreenOffGesturesHandler mScreenOffGesturesHandler;
    private FPScreenOffGesturesHandler mFPScreenOffGesturesHandler;
    private SensorManager mSensorManager;
    private CameraManager mCameraManager;
    private String mRearCameraId;
    private boolean mTorchEnabled;
    private Sensor mProximitySensor;
    private Vibrator mVibrator;
    private int mProximityTimeOut;
    private boolean mProximityWakeSupported;
    private ISearchManager mSearchManagerService;
    private Handler mHandler;
    private int fpTapCounts = 0;
    private boolean fpTapPending = false;
    private boolean screenOffGesturePending = false;
    private boolean fpGesturePending = false;
    private Runnable doubleTapRunnable = new Runnable() {
        public void run() {
            int action = 0;
            if (fpTapCounts > 1) {
                action = str2int(FileUtils.readOneLine(getFPNodeBasedOnScreenState(FP_KEY_DBLTAP_NODE)));
            } else {
                if (isSingleTapEnabledOnFP()) {
                    action = str2int(FileUtils.readOneLine(getFPNodeBasedOnScreenState(FP_KEYS_NODE)));
                }
            }

            if (action != 0) {
                boolean isActionSupported = ArrayUtils.contains(mPowerManager.isScreenOn() ? sFPSupportedActions : sFPSupportedActionsScreenOff, action);
                if (isActionSupported) {
                    fireFPAction(action, true);
                }
            }
            resetDoubleTapOnFP();
        }
    };
    private Runnable screenOffGestureRunnable = new Runnable() {
        public void run() {
            resetScreenOffGestureDelay();
        }
    };
    private Runnable fpGestureRunnable = new Runnable() {
        public void run() {
            resetFPGestureDelay();
        }
    };

    public KeyHandler(Context context) {
        mContext = context;

        mPowerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        mScreenOffGesturesHandler = new ScreenOffGesturesHandler();
        mFPScreenOffGesturesHandler = new FPScreenOffGesturesHandler();

        mGestureWakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                "GestureWakeLock");

        final Resources resources = mContext.getResources();
        mProximityTimeOut = resources.getInteger(
                org.lineageos.platform.internal.R.integer.config_proximityCheckTimeout);
        mProximityWakeSupported = resources.getBoolean(
                org.lineageos.platform.internal.R.bool.config_proximityCheckOnWake);

        if (mProximityWakeSupported) {
            mSensorManager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
            mProximitySensor = mSensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
            mProximityWakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                    "ProximityWakeLock");
        }

        mVibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        if (mVibrator == null || !mVibrator.hasVibrator()) {
            mVibrator = null;
        }

        mCameraManager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        mCameraManager.registerTorchCallback(new MyTorchCallback(), null);

        mHandler = new Handler(Looper.getMainLooper());
    }

    static long[] getLongIntArray(Resources r, int resid) {
        int[] ar = r.getIntArray(resid);
        if (ar == null) {
            return null;
        }
        long[] out = new long[ar.length];
        for (int i = 0; i < ar.length; i++) {
            out[i] = ar[i];
        }
        return out;
    }

    private static ActivityInfo getRunningActivityInfo(Context context) {
        final ActivityManager am = (ActivityManager) context
                .getSystemService(Context.ACTIVITY_SERVICE);
        final PackageManager pm = context.getPackageManager();

        List<ActivityManager.RunningTaskInfo> tasks = am.getRunningTasks(1);
        if (tasks != null && !tasks.isEmpty()) {
            ActivityManager.RunningTaskInfo top = tasks.get(0);
            try {
                return pm.getActivityInfo(top.topActivity, 0);
            } catch (PackageManager.NameNotFoundException e) {
            }
        }
        return null;
    }

    private static void dispatchMediaKeyWithWakeLock(int keycode, Context context) {
        if (ActivityManagerNative.isSystemReady()) {
            KeyEvent event = new KeyEvent(SystemClock.uptimeMillis(),
                    SystemClock.uptimeMillis(), KeyEvent.ACTION_DOWN, keycode, 0);
            MediaSessionLegacyHelper.getHelper(context).sendMediaButtonEvent(event, true);
            event = KeyEvent.changeAction(event, KeyEvent.ACTION_UP);
            MediaSessionLegacyHelper.getHelper(context).sendMediaButtonEvent(event, true);
        }
    }

    private static void switchToLastApp(Context context) {
        final ActivityManager am =
                (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        ActivityManager.RunningTaskInfo lastTask = getLastTask(context, am);

        if (lastTask != null) {
            am.moveTaskToFront(lastTask.id, ActivityManager.MOVE_TASK_NO_USER_ACTION);
        }
    }

    private static ActivityManager.RunningTaskInfo getLastTask(Context context,
                                                               final ActivityManager am) {
        final String defaultHomePackage = resolveCurrentLauncherPackage(context);
        List<ActivityManager.RunningTaskInfo> tasks = am.getRunningTasks(5);

        for (int i = 1; i < tasks.size(); i++) {
            String packageName = tasks.get(i).topActivity.getPackageName();
            if (!packageName.equals(defaultHomePackage)
                    && !packageName.equals(context.getPackageName())
                    && !packageName.equals("com.android.systemui")) {
                return tasks.get(i);
            }
        }
        return null;
    }

    private static String resolveCurrentLauncherPackage(Context context) {
        final Intent launcherIntent = new Intent(Intent.ACTION_MAIN)
                .addCategory(Intent.CATEGORY_HOME);
        final PackageManager pm = context.getPackageManager();
        final ResolveInfo launcherInfo = pm.resolveActivity(launcherIntent, 0);
        return launcherInfo.activityInfo.packageName;
    }

    private String getRearCameraId() {
        if (mRearCameraId == null) {
            try {
                for (final String cameraId : mCameraManager.getCameraIdList()) {
                    CameraCharacteristics characteristics =
                            mCameraManager.getCameraCharacteristics(cameraId);
                    int cOrientation = characteristics.get(CameraCharacteristics.LENS_FACING);
                    if (cOrientation == CameraCharacteristics.LENS_FACING_BACK) {
                        mRearCameraId = cameraId;
                        break;
                    }
                }
            } catch (CameraAccessException e) {
                // Ignore
            }
        }
        return mRearCameraId;
    }

    private Intent getLaunchableIntent(Intent intent) {
        PackageManager pm = mContext.getPackageManager();
        List<ResolveInfo> resInfo = pm.queryIntentActivities(intent, 0);
        if (resInfo.isEmpty()) {
            return null;
        }
        return pm.getLaunchIntentForPackage(resInfo.get(0).activityInfo.packageName);
    }

    private void triggerCameraAction() {
        ensureKeyguardManager();
        WakeLock wl = mPowerManager.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP, "GestureWakeLock");
        wl.acquire(500);
        if (mKeyguardManager.inKeyguardRestrictedInputMode()) {
            launchSecureCamera();
        } else {
            launchCamera();
        }
    }

    private void launchCamera() {
        Intent intent = new Intent(MediaStore.INTENT_ACTION_STILL_IMAGE_CAMERA);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.addFlags(Intent.FLAG_FROM_BACKGROUND);
        if (getBestActivityInfo(intent) != null) {
            // Only launch if we can succeed, but let the user pick the action
            mContext.startActivity(intent);
        }
    }

    private void launchSecureCamera() {
        // Keyguard won't allow a picker, try to pick the secure intent in the package
        // that would be the one used for a default action of launching the camera
        Intent normalIntent = new Intent(MediaStore.INTENT_ACTION_STILL_IMAGE_CAMERA);
        normalIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        normalIntent.addFlags(Intent.FLAG_FROM_BACKGROUND);

        Intent secureIntent = new Intent(MediaStore.INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE);
        secureIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        secureIntent.addFlags(Intent.FLAG_FROM_BACKGROUND);

        ActivityInfo normalActivity = getBestActivityInfo(normalIntent);
        ActivityInfo secureActivity = getBestActivityInfo(secureIntent, normalActivity);
        if (secureActivity != null) {
            secureIntent.setComponent(new ComponentName(secureActivity.applicationInfo.packageName, secureActivity.name));
            mContext.startActivity(secureIntent);
        }
    }

    private ActivityInfo getBestActivityInfo(Intent intent) {
        PackageManager pm = mContext.getPackageManager();
        ResolveInfo resolveInfo = pm.resolveActivity(intent, 0);
        if (resolveInfo != null) {
            return resolveInfo.activityInfo;
        } else {
            // If the resolving failed, just find our own best match
            return getBestActivityInfo(intent, null);
        }
    }

    private ActivityInfo getBestActivityInfo(Intent intent, ActivityInfo match) {
        PackageManager pm = mContext.getPackageManager();
        List<ResolveInfo> activities = pm.queryIntentActivities(intent, 0);
        ActivityInfo best = null;
        if (activities.size() > 0) {
            best = activities.get(0).activityInfo;
            if (match != null) {
                String packageName = match.applicationInfo.packageName;
                for (int i = activities.size() - 1; i >= 0; i--) {
                    ActivityInfo activityInfo = activities.get(i).activityInfo;
                    if (packageName.equals(activityInfo.applicationInfo.packageName)) {
                        best = activityInfo;
                    }
                }
            }
        }
        return best;
    }

    private void openBrowser() {
        mGestureWakeLock.acquire(GESTURE_WAKELOCK_DURATION);
        mPowerManager.wakeUp(SystemClock.uptimeMillis(), GESTURE_WAKEUP_REASON);
        final Intent intent = getLaunchableIntent(
                new Intent(Intent.ACTION_VIEW, Uri.parse("http:")));
        startActivitySafely(intent);
    }

    private void openDialer() {
        mGestureWakeLock.acquire(GESTURE_WAKELOCK_DURATION);
        mPowerManager.wakeUp(SystemClock.uptimeMillis(), GESTURE_WAKEUP_REASON);
        final Intent intent = new Intent(Intent.ACTION_DIAL, null);
        startActivitySafely(intent);
    }

    private void openEmail() {
        mGestureWakeLock.acquire(GESTURE_WAKELOCK_DURATION);
        mPowerManager.wakeUp(SystemClock.uptimeMillis(), GESTURE_WAKEUP_REASON);
        final Intent intent = getLaunchableIntent(
                new Intent(Intent.ACTION_VIEW, Uri.parse("mailto:")));
        startActivitySafely(intent);
    }

    private void openMessages() {
        mGestureWakeLock.acquire(GESTURE_WAKELOCK_DURATION);
        mPowerManager.wakeUp(SystemClock.uptimeMillis(), GESTURE_WAKEUP_REASON);
        final String defaultApplication = Settings.Secure.getString(
                mContext.getContentResolver(), "sms_default_application");
        final PackageManager pm = mContext.getPackageManager();
        final Intent intent = pm.getLaunchIntentForPackage(defaultApplication);
        if (intent != null) {
            startActivitySafely(intent);
        }
    }

    private void toggleFlashlight() {
        String rearCameraId = getRearCameraId();
        if (rearCameraId != null) {
            mGestureWakeLock.acquire(GESTURE_WAKELOCK_DURATION);
            try {
                mCameraManager.setTorchMode(rearCameraId, !mTorchEnabled);
                mTorchEnabled = !mTorchEnabled;
            } catch (CameraAccessException e) {
                // Ignore
            }
        }
    }

    private void ensureKeyguardManager() {
        if (mKeyguardManager == null) {
            mKeyguardManager =
                    (KeyguardManager) mContext.getSystemService(Context.KEYGUARD_SERVICE);
        }
    }

    private void resetDoubleTapOnFP() {
        fpTapCounts = 0;
        fpTapPending = false;
        mHandler.removeCallbacks(doubleTapRunnable);
    }

    private void detectDoubleTapOnFP() {
        fpTapCounts++;
        if (fpTapCounts == 1 || fpTapCounts == 2) {
            doHapticFeedbackFP(false);
        }
        if (!fpTapPending) {
            fpTapPending = true;
            mHandler.postDelayed(doubleTapRunnable, ViewConfiguration.getDoubleTapTimeout());
        }
    }

    private boolean isSingleTapEnabledOnFP() {
        return !FileUtils.readOneLine(getFPNodeBasedOnScreenState(FP_KEYS_NODE)).equals("0");
    }

    private boolean isDoubleTapEnabledOnFP() {
        return !FileUtils.readOneLine(getFPNodeBasedOnScreenState(FP_KEY_DBLTAP_NODE)).equals("0");
    }

    private boolean isHapticFeedbackEnabledOnFP() {
        return !FileUtils.readOneLine(getFPNodeBasedOnScreenState(FP_HAPTIC_NODE)).equals("0");
    }

    private String getFPNodeBasedOnScreenState(String node) {
        if (mPowerManager.isScreenOn()) {
            return node;
        }
        switch (node) {
            case FP_KEYS_NODE:
                return FP_KEYS_SCREENOFF_NODE;
            case FP_HAPTIC_NODE:
                return FP_HAPTIC_SCREENOFF_NODE;
            case FP_KEY_DBLTAP_NODE:
                return FP_KEY_SCREENOFF_DBLTAP_NODE;
            case FP_KEY_HOLD_NODE:
                return FP_KEY_SCREENOFF_HOLD_NODE;
            case FP_KEY_RIGHT_NODE:
                return FP_KEY_SCREENOFF_RIGHT_NODE;
            case FP_KEY_LEFT_NODE:
                return FP_KEY_SCREENOFF_LEFT_NODE;
        }
        return node;
    }

    public boolean handleKeyEvent(KeyEvent event) {
        int scanCode = event.getScanCode();

        if (DEBUG) {
            Log.d(TAG, "DEBUG: action=" + event.getAction()
                    + ", flags=" + event.getFlags()
                    + ", keyCode=" + event.getKeyCode()
                    + ", scanCode=" + event.getScanCode()
                    + ", metaState=" + event.getMetaState()
                    + ", repeatCount=" + event.getRepeatCount());
        }

        boolean isFPScanCode = ArrayUtils.contains(sSupportedFPGestures, scanCode);
        boolean isScreenOffGesturesScanCode = ArrayUtils.contains(sSupportedScreenOffGestures, scanCode);
        if (!isFPScanCode && !isScreenOffGesturesScanCode) {
            return false;
        }

        boolean isFPGestureEnabled = FileUtils.readOneLine(FP_HOME_NODE).equals("1");
        boolean isFPGestureEnabledOnScreenOff = FileUtils.readOneLine(FP_HOME_OFF_NODE).equals("1");

        boolean isScreenOn = mPowerManager.isScreenOn();

        // We only want ACTION_UP event
        if (event.getAction() != KeyEvent.ACTION_UP) {
            return true;
        }
        
        if (isFPScanCode){
            if (fpGesturePending) {
                return false;
            } else {
                resetFPGestureDelay();
                fpGesturePending = true;
                mHandler.postDelayed(fpGestureRunnable, 10);
            }
        }

        if (scanCode != FP_TAP_SCANCODE) {
            resetDoubleTapOnFP();
        }

        if (isFPScanCode) {
            if ((!isFPGestureEnabled) || (!isScreenOn && !isFPGestureEnabledOnScreenOff)) {
                resetDoubleTapOnFP();
                return false;
            }
            if (!isScreenOn && isFPGestureEnabledOnScreenOff) {
                processFPScreenOffScancode(scanCode);
            } else {
                processFPScancode(scanCode);
            }
        } else if (isScreenOffGesturesScanCode) {
            processScreenOffScancode(scanCode);
        }
        return true;
    }

    private void processFPScancode(int scanCode) {
        int action = 0;
        boolean isScreenOn = mPowerManager.isScreenOn();
        switch (scanCode) {
            case FP_TAP_SCANCODE:
                if (isDoubleTapEnabledOnFP()) {
                    detectDoubleTapOnFP();
                    return;
                } else {
                    resetDoubleTapOnFP();
                    action = str2int(FileUtils.readOneLine(getFPNodeBasedOnScreenState(FP_KEYS_NODE)));
                }
                break;
            case FP_HOLD_SCANCODE:
                action = str2int(FileUtils.readOneLine(getFPNodeBasedOnScreenState(FP_KEY_HOLD_NODE)));
                break;
            case FP_RIGHT_SCANCODE:
                action = str2int(FileUtils.readOneLine(getFPNodeBasedOnScreenState(FP_KEY_RIGHT_NODE)));
                break;
            case FP_LEFT_SCANCODE:
                action = str2int(FileUtils.readOneLine(getFPNodeBasedOnScreenState(FP_KEY_LEFT_NODE)));
                break;
        }
        boolean isActionSupported = ArrayUtils.contains(isScreenOn ? sFPSupportedActions : sFPSupportedActionsScreenOff, action);
        if (isActionSupported) {
            fireFPAction(action, false);
        }
    }

    private void fireFPAction(int action, boolean isDoubleTap) {
        ensureKeyguardManager();
        boolean isHapticFeedbackEnabledOnFP = isHapticFeedbackEnabledOnFP();
        if (isDoubleTap && action != ACTION_CAMERA && action != ACTION_FLASHLIGHT) {
            isHapticFeedbackEnabledOnFP = false;
        }
        if (isHapticFeedbackEnabledOnFP && (action == ACTION_CAMERA || action == ACTION_FLASHLIGHT)) {
            vibrate(action == ACTION_CAMERA ? 500 : 250);
        }
        if (isHapticFeedbackEnabledOnFP && action == ACTION_POWER) {
            doHapticFeedbackFP(false);
        }
        switch (action) {
            case ACTION_HOME:
                if (!mKeyguardManager.inKeyguardRestrictedInputMode()) {
                    triggerVirtualKeypress(mHandler, KeyEvent.KEYCODE_HOME);
                }
                break;
            case ACTION_POWER:
                toggleScreenState();
                break;
            case ACTION_BACK:
                triggerVirtualKeypress(mHandler, KeyEvent.KEYCODE_BACK);
                break;
            case ACTION_RECENTS:
                if (!mKeyguardManager.inKeyguardRestrictedInputMode()) {
                    triggerVirtualKeypress(mHandler, KeyEvent.KEYCODE_APP_SWITCH);
                }
                break;
            case ACTION_VOLUME_UP:
                triggerVirtualKeypress(mHandler, KeyEvent.KEYCODE_VOLUME_UP);
                break;
            case ACTION_VOLUME_DOWN:
                triggerVirtualKeypress(mHandler, KeyEvent.KEYCODE_VOLUME_DOWN);
                break;
            case ACTION_VOICE_ASSISTANT:
                if (!mKeyguardManager.inKeyguardRestrictedInputMode()) {
                    fireGoogleNowOnTap();
                }
                return;
            case ACTION_PLAY_PAUSE:
                dispatchMediaKeyWithWakeLock(KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE, mContext);
                break;
            case ACTION_PREVIOUS_TRACK:
                dispatchMediaKeyWithWakeLock(KeyEvent.KEYCODE_MEDIA_PREVIOUS, mContext);
                break;
            case ACTION_NEXT_TRACK:
                dispatchMediaKeyWithWakeLock(KeyEvent.KEYCODE_MEDIA_NEXT, mContext);
                break;
            case ACTION_FLASHLIGHT:
                toggleFlashlight();
                break;
            case ACTION_CAMERA:
                triggerCameraAction();
                break;
            case ACTION_SCREENSHOT:
                triggerVirtualKeypress(mHandler, KeyEvent.KEYCODE_SYSRQ);
                break;
            case ACTION_PIP:
                if (!mKeyguardManager.inKeyguardRestrictedInputMode()) {
                    goToPipMode();
                }
                break;
            case ACTION_LAST_APP:
                if (!mKeyguardManager.inKeyguardRestrictedInputMode()) {
                    switchToLastApp(mContext);
                }
                break;
        }
        if (isHapticFeedbackEnabledOnFP && action != ACTION_VOICE_ASSISTANT && action != ACTION_CAMERA && action != ACTION_FLASHLIGHT && action != ACTION_POWER) { // prevent double vibration
            doHapticFeedbackFP(false);
        }
    }

    private void vibrate(int intensity) {
        if (mVibrator == null) {
            return;
        }
        mVibrator.vibrate(intensity);
    }

    private void goToPipMode() {
        ActivityInfo ai = getRunningActivityInfo(mContext);
        if (ai != null && !ai.supportsPictureInPicture()) {
            try {
                PackageManager pm = mContext.getPackageManager();
                Resources resources = pm.getResourcesForApplication("org.lineageos.settings.device");
                int resId = resources.getIdentifier("app_does_not_support_pip", "string", "org.lineageos.settings.device");
                final String text = resources.getString(resId);
                mHandler.post(new Runnable() {
                    public void run() {
                        Toast.makeText(mContext, text, Toast.LENGTH_SHORT).show();
                    }
                });
            } catch (Exception e) {
            }
            return;
        }
        triggerVirtualKeypress(mHandler, 171);
    }

    private void toggleScreenState() {
        if (mPowerManager.isScreenOn()) {
            mPowerManager.goToSleep(SystemClock.uptimeMillis());
        } else {
            mPowerManager.wakeUp(SystemClock.uptimeMillis());
        }
    }

    private void triggerVirtualKeypress(final Handler handler, final int keyCode) {
        final InputManager im = InputManager.getInstance();
        long now = SystemClock.uptimeMillis();

        final KeyEvent downEvent = new KeyEvent(now, now, KeyEvent.ACTION_DOWN,
                keyCode, 0, 0, KeyCharacterMap.VIRTUAL_KEYBOARD, 0,
                KeyEvent.FLAG_FROM_SYSTEM, InputDevice.SOURCE_CLASS_BUTTON);
        final KeyEvent upEvent = KeyEvent.changeAction(downEvent,
                KeyEvent.ACTION_UP);

        // add a small delay to make sure everything behind got focus
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                im.injectInputEvent(downEvent, InputManager.INJECT_INPUT_EVENT_MODE_ASYNC);
            }
        }, 10);

        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                im.injectInputEvent(upEvent, InputManager.INJECT_INPUT_EVENT_MODE_ASYNC);
            }
        }, 20);
    }

    private void fireGoogleNowOnTap() {
        doHapticFeedbackFP(true);
        mSearchManagerService = ISearchManager.Stub.asInterface(ServiceManager.getService(Context.SEARCH_SERVICE));
        if (mSearchManagerService != null) {
            try {
                mSearchManagerService.launchAssist(new Bundle());
            } catch (RemoteException e) {
            }
        }
    }

    private int str2int(String str) {
        if (str == null || str.isEmpty()) {
            return 0;
        }
        try {
            return Integer.valueOf(str);
        } catch (Exception e) {
            return 0;
        }
    }

    private void processFPScreenOffScancode(int scanCode) {
        if (!mFPScreenOffGesturesHandler.hasMessages(FP_ACTION_REQUEST)) {
            Message msg = mFPScreenOffGesturesHandler.obtainMessage(FP_ACTION_REQUEST);
            msg.arg1 = scanCode;
            boolean defaultProximity = mContext.getResources().getBoolean(
                    org.lineageos.platform.internal.R.bool.config_proximityCheckOnWakeEnabledByDefault);
            boolean proximityWakeCheckEnabled = LineageSettings.System.getInt(mContext.getContentResolver(),
                    LineageSettings.System.PROXIMITY_ON_WAKE, defaultProximity ? 1 : 0) == 1;
            if (mProximityWakeSupported && proximityWakeCheckEnabled && mProximitySensor != null) {
                mFPScreenOffGesturesHandler.sendMessageDelayed(msg, mProximityTimeOut);
                registerFPScreenOffListener(scanCode);
            } else {
                mFPScreenOffGesturesHandler.sendMessage(msg);
            }
        }
    }

    private void registerFPScreenOffListener(final int scanCode) {
        mProximityWakeLock.acquire();
        mSensorManager.registerListener(new SensorEventListener() {
            @Override
            public void onSensorChanged(SensorEvent event) {
                mProximityWakeLock.release();
                mSensorManager.unregisterListener(this);
                if (!mFPScreenOffGesturesHandler.hasMessages(FP_ACTION_REQUEST)) {
                    // The sensor took to long, ignoring.
                    return;
                }
                mFPScreenOffGesturesHandler.removeMessages(FP_ACTION_REQUEST);
                if (event.values[0] == mProximitySensor.getMaximumRange()) {
                    Message msg = mFPScreenOffGesturesHandler.obtainMessage(FP_ACTION_REQUEST);
                    msg.arg1 = scanCode;
                    mFPScreenOffGesturesHandler.sendMessage(msg);
                }
            }

            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
            }

        }, mProximitySensor, SensorManager.SENSOR_DELAY_FASTEST);
    }


    private void resetScreenOffGestureDelay() {
        screenOffGesturePending = false;
        mHandler.removeCallbacks(screenOffGestureRunnable);
    }

    private void resetFPGestureDelay() {
        fpGesturePending = false;
        mHandler.removeCallbacks(fpGestureRunnable);
    }

    private void processScreenOffScancode(int scanCode) {
        if (screenOffGesturePending) {
            return;
        } else {
            resetScreenOffGestureDelay();
            screenOffGesturePending = true;
            mHandler.postDelayed(screenOffGestureRunnable, 500);
        }
        if (!mScreenOffGesturesHandler.hasMessages(GESTURE_REQUEST)) {
            Message msg = mScreenOffGesturesHandler.obtainMessage(GESTURE_REQUEST);
            msg.arg1 = scanCode;
            boolean defaultProximity = mContext.getResources().getBoolean(
                    org.lineageos.platform.internal.R.bool.config_proximityCheckOnWakeEnabledByDefault);
            boolean proximityWakeCheckEnabled = LineageSettings.System.getInt(mContext.getContentResolver(),
                    LineageSettings.System.PROXIMITY_ON_WAKE, defaultProximity ? 1 : 0) == 1;
            if (mProximityWakeSupported && proximityWakeCheckEnabled && mProximitySensor != null) {
                mScreenOffGesturesHandler.sendMessageDelayed(msg, mProximityTimeOut);
                registerScreenOffGesturesListener(scanCode);
            } else {
                mScreenOffGesturesHandler.sendMessage(msg);
            }
        }
    }

    private void registerScreenOffGesturesListener(final int scanCode) {
        mProximityWakeLock.acquire();
        mSensorManager.registerListener(new SensorEventListener() {
            @Override
            public void onSensorChanged(SensorEvent event) {
                mProximityWakeLock.release();
                mSensorManager.unregisterListener(this);
                if (!mScreenOffGesturesHandler.hasMessages(GESTURE_REQUEST)) {
                    // The sensor took to long, ignoring.
                    return;
                }
                mScreenOffGesturesHandler.removeMessages(GESTURE_REQUEST);
                if (event.values[0] == mProximitySensor.getMaximumRange()) {
                    Message msg = mScreenOffGesturesHandler.obtainMessage(GESTURE_REQUEST);
                    msg.arg1 = scanCode;
                    mScreenOffGesturesHandler.sendMessage(msg);
                }
            }

            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
            }

        }, mProximitySensor, SensorManager.SENSOR_DELAY_FASTEST);
    }

    private void fireScreenOffAction(int action) {
        boolean haptic = Settings.System.getInt(mContext.getContentResolver(), KEY_GESTURE_ENABLE_HAPTIC_FEEDBACK, 1) != 0;
        if (haptic && (action == ACTION_CAMERA || action == ACTION_FLASHLIGHT)) {
            vibrate(action == ACTION_CAMERA ? 500 : 250);
        }
        if (haptic && action == ACTION_POWER){
            doHapticFeedbackScreenOff();
        }
        switch (action) {
            case ACTION_POWER:
                toggleScreenState();
                break;
            case ACTION_PLAY_PAUSE:
                dispatchMediaKeyWithWakeLock(KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE, mContext);
                break;
            case ACTION_PREVIOUS_TRACK:
                dispatchMediaKeyWithWakeLock(KeyEvent.KEYCODE_MEDIA_PREVIOUS, mContext);
                break;
            case ACTION_NEXT_TRACK:
                dispatchMediaKeyWithWakeLock(KeyEvent.KEYCODE_MEDIA_NEXT, mContext);
                break;
            case ACTION_FLASHLIGHT:
                toggleFlashlight();
                break;
            case ACTION_CAMERA:
                triggerCameraAction();
                break;
            case ACTION_BROWSER:
                openBrowser();
                break;
            case ACTION_DIALER:
                openDialer();
                break;
            case ACTION_EMAIL:
                openEmail();
                break;
            case ACTION_MESSAGES:
                openMessages();
                break;
        }
        if (action != ACTION_FLASHLIGHT && action != ACTION_CAMERA && action != ACTION_POWER) {
            doHapticFeedbackScreenOff();
        }
    }

    private void startActivitySafely(Intent intent) {
        intent.addFlags(
                Intent.FLAG_ACTIVITY_NEW_TASK
                        | Intent.FLAG_ACTIVITY_SINGLE_TOP
                        | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        try {
            UserHandle user = new UserHandle(UserHandle.USER_CURRENT);
            mContext.startActivityAsUser(intent, null, user);
        } catch (ActivityNotFoundException e) {
            // Ignore
        }
    }

    private void doHapticFeedbackScreenOff() {
        if (mVibrator == null) {
            return;
        }
        boolean enabled = Settings.System.getInt(mContext.getContentResolver(), KEY_GESTURE_ENABLE_HAPTIC_FEEDBACK, 1) != 0;
        if (enabled) {
            mVibrator.vibrate(50);
        }
    }

    private void doHapticFeedbackFP(boolean longpress) {
        if (mVibrator == null) {
            return;
        }

        if (isHapticFeedbackEnabledOnFP()) {
            int owningUid;
            String owningPackage;
            owningUid = android.os.Process.myUid();
            owningPackage = mContext.getOpPackageName();
            VibrationEffect effect = longpress ? VibrationEffect.createWaveform(LONG_PRESS_VIBRATION_PATTERN, -1) : VibrationEffect.get(VibrationEffect.EFFECT_CLICK);
            mVibrator.vibrate(owningUid, owningPackage, effect, VIBRATION_ATTRIBUTES);
        }
    }

    private class ScreenOffGesturesHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            int scanCode = msg.arg1;
            int action = 0;
            switch (scanCode) {
                case GESTURE_SWIPE_RIGHT_SCANCODE:
                    action = str2int(FileUtils.readOneLine(GESTURE_SWIPE_RIGHT_NODE));
                    break;
                case GESTURE_SWIPE_LEFT_SCANCODE:
                    action = str2int(FileUtils.readOneLine(GESTURE_SWIPE_LEFT_NODE));
                    break;
                case GESTURE_SWIPE_DOWN_SCANCODE:
                    action = str2int(FileUtils.readOneLine(GESTURE_SWIPE_DOWN_NODE));
                    break;
                case GESTURE_SWIPE_UP_SCANCODE:
                    action = str2int(FileUtils.readOneLine(GESTURE_SWIPE_UP_NODE));
                    break;
                case GESTURE_DOUBLE_TAP_SCANCODE:
                    action = str2int(FileUtils.readOneLine(GESTURE_DOUBLE_TAP_NODE));
                    if (action != 0) {
                        action = ACTION_POWER;
                    }
                    break;
            }
            boolean isActionSupported = ArrayUtils.contains(sScreenOffSupportedActions, action);
            if (isActionSupported) {
                fireScreenOffAction(action);
            }
        }
    }

    private class FPScreenOffGesturesHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            processFPScancode(msg.arg1);
        }
    }

    private class MyTorchCallback extends CameraManager.TorchCallback {
        @Override
        public void onTorchModeChanged(String cameraId, boolean enabled) {
            if (!cameraId.equals(mRearCameraId))
                return;
            mTorchEnabled = enabled;
        }

        @Override
        public void onTorchModeUnavailable(String cameraId) {
            if (!cameraId.equals(mRearCameraId))
                return;
            mTorchEnabled = false;
        }
    }
}

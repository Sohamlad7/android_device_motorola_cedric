/*
 * Copyright (C) 2014 The CyanogenMod Project
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

import android.app.ActivityThread;
import android.os.IBinder;
import android.os.Parcel;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Slog;

import com.android.server.LocalServices;
import com.android.server.display.DisplayTransformManager;
import static com.android.server.display.DisplayTransformManager.LEVEL_COLOR_MATRIX_NIGHT_DISPLAY;

import org.lineageos.internal.util.FileUtils;

public class DisplayColorCalibration {

    private static final String TAG = "DisplayColorCalibration";

    private static final String COLOR_FILE = "/sys/class/graphics/fb0/rgb";

    private static final int LEVEL_COLOR_MATRIX_LIVEDISPLAY = LEVEL_COLOR_MATRIX_NIGHT_DISPLAY + 1;

    private static final int MODE_UNSUPPORTED          = 0;
    private static final int MODE_HWC2_COLOR_TRANSFORM = 1;
    private static final int MODE_SYSFS_RGB            = 2;

    private static final int sMode;

    private static final int MIN = 255;
    private static final int MAX = 32768;

    private static final int[] sCurColors = new int[] { MAX, MAX, MAX };

    private static DisplayTransformManager sDTMService;

    static {
        // Determine mode of operation.
        // Order of priority is:
        // 1) HWC2 color transform
        // 2) sysfs rgb file
        if (ActivityThread.currentApplication().getApplicationContext().getResources().getBoolean(
                    com.android.internal.R.bool.config_setColorTransformAccelerated)) {
            sMode = MODE_HWC2_COLOR_TRANSFORM;
        } else if (FileUtils.isFileWritable(COLOR_FILE)) {
            sMode = MODE_SYSFS_RGB;
        } else {
            sMode = MODE_UNSUPPORTED;
        }
    }

    public static boolean isSupported() {
        return sMode != MODE_UNSUPPORTED;
    }

    public static int getMaxValue()  {
        return MAX;
    }

    public static int getMinValue()  {
        return MIN;
    }

    public static int getDefValue() {
        return getMaxValue();
    }

    public static String getCurColors()  {
        if (sMode == MODE_SYSFS_RGB) {
            return FileUtils.readOneLine(COLOR_FILE);
        }

        return String.format("%d %d %d", sCurColors[0],
                sCurColors[1], sCurColors[2]);
    }

    public static boolean setColors(String colors) {
        if (sMode == MODE_SYSFS_RGB) {
            return FileUtils.writeLine(COLOR_FILE, colors);
        } else if (sMode == MODE_HWC2_COLOR_TRANSFORM) {
            if (sDTMService == null) {
                sDTMService = LocalServices.getService(DisplayTransformManager.class);
                if (sDTMService == null) {
                    return false;
                }
            }
            sDTMService.setColorMatrix(LEVEL_COLOR_MATRIX_LIVEDISPLAY, toColorMatrix(colors));
            return true;
        }
        return false;
    }

    private static float[] toColorMatrix(String rgbString) {
        String[] adj = rgbString == null ? null : rgbString.split(" ");

        if (adj == null || adj.length != 3) {
            return null;
        }

        float[] mat = new float[16];

        // sanity check
        for (int i = 0; i < 3; i++) {
            int v = Integer.parseInt(adj[i]);

            if (v >= MAX) {
                v = MAX;
            } else if (v < MIN) {
                v = MIN;
            }

            mat[i * 5] = (float)v / (float)MAX;
            sCurColors[i] = v;
        }

        mat[15] = 1.0f;
        return mat;
    }
}

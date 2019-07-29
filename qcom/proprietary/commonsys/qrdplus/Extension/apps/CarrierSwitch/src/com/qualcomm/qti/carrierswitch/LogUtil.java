/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch;

import android.util.Log;

public class LogUtil {

    private static final String TAG = "CarrierSwitch";
    private static final boolean DEBUG_V = false;
    private static final boolean DEBUG_D = true;
    private static final boolean DEBUG_E = true;

    public static void v(Class c, String message) {
        if (DEBUG_V) Log.v(TAG, c.getSimpleName() + ": " + message);
    }

    public static void v(Object o, String message) {
        if (DEBUG_V) Log.v(TAG, o.getClass().getSimpleName() + ": " + message);
    }

    public static void d(Class c, String message) {
        if (DEBUG_D) Log.d(TAG, c.getSimpleName() + ": " + message);
    }

    public static void d(Object o, String message) {
        if (DEBUG_D) Log.d(TAG, o.getClass().getSimpleName() + ": " + message);
    }

    public static void e(Class c, String message) {
        if (DEBUG_E) Log.e(TAG, c.getSimpleName() + ": " + message);
    }

    public static void e(Object o, String message) {
        if (DEBUG_E) Log.e(TAG, o.getClass().getSimpleName() + ": " + message);
    }
}

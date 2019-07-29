/*
 * Copyright (c) 2015,2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2011-2014, The Linux Foundation. All rights reserved.
 */

package com.qualcomm.qti;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageInfo;
import android.os.Binder;
import android.os.IBinder;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.Trace;
import android.util.Log;
import java.util.ArrayList;

public class Performance
{
    private static final String TAG = "Perf";
    private static final String PERF_SERVICE_BINDER_NAME = "vendor.perfservice";
    private static final boolean sPerfServiceDisabled = false;
    private static final boolean DEBUG = false;

    private static IBinder clientBinder;
    private static IBinder sPerfServiceBinder;
    private static IPerfManager sPerfService;
    private static boolean sLoaded = false;
    private final Object mLock = new Object();
    private static PerfServiceDeathRecipient sPerfServiceDeathRecipient;
    private static boolean sIsPlatformOrPrivApp = true;
    private static boolean sIsUntrustedDomain = false;
    private int UXE_EVENT_BINDAPP = 2;
    private static Boolean sIsChecked = false;

    static {
        try {
            System.loadLibrary("qti_performance");
        } catch (UnsatisfiedLinkError e) {
        }
    }
    /** @hide */
    public Performance() {
    }

    /** @hide */
    public Performance(Context context) {
        if (DEBUG)Trace.traceBegin(Trace.TRACE_TAG_ALWAYS, "Create Performance instance");
        synchronized (Performance.class) {
            if (!sLoaded) {
                connectPerfServiceLocked();
                if (sPerfService == null && !sPerfServiceDisabled)
                    Log.e(TAG, "Perf service is unavailable.");
                else
                    sLoaded = true;
            }
        }
        checkAppPlatformSigned(context);
        if (DEBUG)Trace.traceEnd(Trace.TRACE_TAG_ALWAYS);
    }

    /** @hide */
    public Performance(boolean isUntrusterdDomain) {
        sIsUntrustedDomain = isUntrusterdDomain;
    }

    /* The following are the PerfLock API return values*/
    /** @hide */ public static final int REQUEST_FAILED = -1;
    /** @hide */ public static final int REQUEST_SUCCEEDED = 0;

    /** @hide */ private int mHandle = 0;


    private void connectPerfServiceLocked() {
        if (sPerfService != null || sPerfServiceDisabled) return;

        if (DEBUG)Trace.traceBegin(Trace.TRACE_TAG_ALWAYS, "connectPerfServiceLocked");
        Log.i(TAG, "Connecting to perf service.");

        sPerfServiceBinder = ServiceManager.getService(PERF_SERVICE_BINDER_NAME);
        if (sPerfServiceBinder == null) {
            Log.e(TAG, "Perf service is now down, set sPerfService as null.");
            if (DEBUG)Trace.traceEnd(Trace.TRACE_TAG_ALWAYS);
            return;
        }
        try {
            sPerfServiceDeathRecipient = new PerfServiceDeathRecipient();
            //link perfDeathRecipient to binder to receive DeathRecipient call back.
            sPerfServiceBinder.linkToDeath(sPerfServiceDeathRecipient, 0);
        } catch (RemoteException e) {
            Log.e(TAG, "Perf service is now down, leave sPerfService as null.");
            if (DEBUG)Trace.traceEnd(Trace.TRACE_TAG_ALWAYS);
            return;
        }
        if (sPerfServiceBinder != null)
            sPerfService = IPerfManager.Stub.asInterface(sPerfServiceBinder);

        if (sPerfService != null) {
            try {
                clientBinder = new Binder();
                sPerfService.setClientBinder(clientBinder);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
        if (DEBUG)Trace.traceEnd(Trace.TRACE_TAG_ALWAYS);
    }

    /* The following functions are the PerfLock APIs*/
    /** @hide */
    public int perfLockAcquire(int duration, int... list) {
        if (sIsPlatformOrPrivApp && !sIsUntrustedDomain) {
            mHandle = native_perf_lock_acq(mHandle, duration, list);
        } else {
            synchronized (mLock) {
                try {
                    if (sPerfService != null)
                        mHandle = sPerfService.perfLockAcquire(duration, list);
                    else
                        return REQUEST_FAILED;

                } catch (RemoteException e) {
                    Log.e(TAG, "Error calling perfLockAcquire", e);
                    return REQUEST_FAILED;
                }
            }
        }
        if (mHandle <= 0)
            return REQUEST_FAILED;
        else
            return mHandle;
    }

    /** @hide */
    public int perfLockRelease() {
        int retValue = REQUEST_SUCCEEDED;
        if (sIsPlatformOrPrivApp && !sIsUntrustedDomain) {
            retValue = native_perf_lock_rel(mHandle);
            mHandle = 0;
            return retValue;
        } else {
            synchronized (mLock) {
                try {
                    if (sPerfService != null)
                        retValue = sPerfService.perfLockReleaseHandler(mHandle);
                    else
                        retValue = REQUEST_FAILED;
                } catch (RemoteException e) {
                    Log.e(TAG, "Error calling perfLockRelease", e);
                    return REQUEST_FAILED;
                }
            }
            return retValue;
        }
    }

    /** @hide */
    public int perfLockReleaseHandler(int _handle) {
        if (sIsPlatformOrPrivApp && !sIsUntrustedDomain) {
            return native_perf_lock_rel(_handle);
        } else {
            int retValue = REQUEST_SUCCEEDED;
            synchronized (mLock) {
                try {
                    if (sPerfService != null)
                        retValue = sPerfService.perfLockReleaseHandler(_handle);
                    else
                        retValue = REQUEST_FAILED;
                } catch (RemoteException e) {
                    Log.e(TAG, "Error calling perfLockRelease(handle)", e);
                    return REQUEST_FAILED;
                }
            }
            return retValue;
        }
    }

    /** @hide */
    public int perfHint(int hint, String userDataStr, int userData1, int userData2) {
        if (sIsPlatformOrPrivApp && !sIsUntrustedDomain) {
            mHandle = native_perf_hint(hint, userDataStr, userData1, userData2);
        } else {
            synchronized (mLock) {
                try {
                    if (sPerfService != null)
                        mHandle = sPerfService.perfHint(hint, userDataStr, userData1, userData2,
                                Process.myTid());
                    else
                        return REQUEST_FAILED;
                } catch (RemoteException e) {
                    Log.e(TAG, "Error calling perfHint", e);
                    return REQUEST_FAILED;
                }
            }
        }
        if (mHandle <= 0)
            return REQUEST_FAILED;
        else
            return mHandle;
    }

    /** @hide */
    public int perfGetFeedback(int req, String userDataStr) {
        return native_perf_get_feedback(req, userDataStr);
    }

    public int perfIOPrefetchStart(int PId, String Pkg_name, String Code_path)
    {
        return native_perf_io_prefetch_start(PId,Pkg_name, Code_path);
    }

    public int perfIOPrefetchStop(){
        return native_perf_io_prefetch_stop();
    }

    public int perfUXEngine_events(int opcode, int pid, String pkg_name, int lat)
    {
        if (opcode == UXE_EVENT_BINDAPP) {
            synchronized (mLock) {
                try {
                    if (sPerfService != null)
                        mHandle = sPerfService.perfUXEngine_events(opcode, pid, pkg_name, lat);
                    else
                        return REQUEST_FAILED;
                } catch (RemoteException e) {
                    Log.e(TAG, "Error calling perfHint", e);
                    return REQUEST_FAILED;
                }
            }
        } else {
            mHandle = native_perf_uxEngine_events(opcode, pid, pkg_name, lat);
        }
        if (mHandle <= 0)
            return REQUEST_FAILED;
        else
            return mHandle;
    }

    public String perfUXEngine_trigger(int opcode)
    {
        return native_perf_uxEngine_trigger(opcode);
    }

    private void checkAppPlatformSigned(Context context) {
        synchronized(sIsChecked){
            if (context == null || sIsChecked)return;
            if (DEBUG)Trace.traceBegin(Trace.TRACE_TAG_ALWAYS, "checkAppPlatformSigned");
            try {
                PackageInfo pkg = context.getPackageManager().getPackageInfo(
                        context.getPackageName(), PackageManager.GET_SIGNATURES);
                PackageInfo sys = context.getPackageManager().getPackageInfo(
                        "android", PackageManager.GET_SIGNATURES);
                sIsPlatformOrPrivApp =
                        (pkg != null && pkg.signatures != null
                        && pkg.signatures.length > 0
                        && sys.signatures[0].equals(pkg.signatures[0]))
                        || pkg.applicationInfo.isPrivilegedApp();
            } catch (PackageManager.NameNotFoundException e) {
                Log.e(TAG, "packageName is not found.");
                sIsPlatformOrPrivApp = true;
            }
            sIsChecked = true;
            if (DEBUG)Log.d(TAG, "sIsPlatformOrPrivApp is " + sIsPlatformOrPrivApp);
            if (DEBUG)Trace.traceEnd(Trace.TRACE_TAG_ALWAYS);
        }
    }

    private final class PerfServiceDeathRecipient implements IBinder.DeathRecipient {
        public void binderDied() {
            synchronized(mLock) {
                Log.e(TAG, "Perf Service died.");
                if (sPerfServiceBinder != null)
                    sPerfServiceBinder.unlinkToDeath(this, 0);
                sPerfServiceBinder = null;
                sPerfService = null;
            }
        }
    }

    private native int  native_perf_lock_acq(int handle, int duration, int list[]);
    private native int  native_perf_lock_rel(int handle);
    private native int  native_perf_hint(int hint, String userDataStr, int userData1, int userData2);
    private native int  native_perf_get_feedback(int req, String userDataStr);
    private native int  native_perf_io_prefetch_start(int pid, String pkg_name, String Code_path);
    private native int  native_perf_io_prefetch_stop();
    private native int  native_perf_uxEngine_events(int opcode, int pid, String pkg_name, int lat);
    private native String  native_perf_uxEngine_trigger(int opcode);
}

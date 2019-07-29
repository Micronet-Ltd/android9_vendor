/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qti.xdivert;

import android.app.Service;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;

import com.android.internal.telephony.TelephonyIntents;

public class XDivertService extends Service {

    private XDivertUtility mXDivertUtility;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mXDivertUtility);
    }

    @Override
    public int onStartCommand(Intent intent, int id, int startId) {
        mXDivertUtility = XDivertUtility.init(this);
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED);
        filter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        this.registerReceiver(mXDivertUtility, filter);
        return START_STICKY;
    }
}


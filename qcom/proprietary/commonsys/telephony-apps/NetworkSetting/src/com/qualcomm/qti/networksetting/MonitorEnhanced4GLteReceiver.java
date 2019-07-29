/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.networksetting;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.android.internal.telephony.TelephonyIntents;

public class MonitorEnhanced4GLteReceiver extends BroadcastReceiver {
    private static final String TAG = "MonitorEhanced4GLteReceiver";
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "Received action: " + intent.getAction());
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction()) ||
                Intent.ACTION_LOCKED_BOOT_COMPLETED.equals(intent.getAction()) ||
                TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
            //only for CT mode to start this feature.
            if (context.getResources().
                    getBoolean(R.bool.config_enhanced4glte_status)) {
                Log.d(TAG, "start Enhanced4GLteStatusService");
                context.startService(new Intent(context, Enhanced4GLteStatusService.class));
            }
       }
    }
}

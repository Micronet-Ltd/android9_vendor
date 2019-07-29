/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/**
 * Receive boot completed broadcast and start {@link CarrierSwitchDeamonService}
 * to check if current carrier match to expected carrier.
 */
public class BootupReceiver extends BroadcastReceiver {
    private final String ACTION_BOOT = "android.intent.action.BOOT_COMPLETED";
    @Override
    public void onReceive(Context context, Intent intent) {
        if (ACTION_BOOT.equals(intent.getAction())) {
            // TODO
            LogUtil.d(this, "onReceive BOOT_COMPLETED");
        }
    }
}

/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qti.xdivert;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;
import android.telephony.TelephonyManager;
import android.util.Log;

public class XDivert extends BroadcastReceiver {

    private static final boolean DBG = true;
    private static final String LOG_TAG = "XDivert";

    @Override
    public void onReceive(Context context, Intent intent) {
        // Return if not a primary user
        if (UserHandle.myUserId() != 0) {
            return;
        }
        Log.v(LOG_TAG,"Action intent recieved:"+intent);
        if (TelephonyManager.getDefault().getMultiSimConfiguration() ==
                TelephonyManager.MultiSimVariants.DSDS){
            Intent serviceIntent = new Intent(context, XDivertService.class);
            context.startService(serviceIntent);
        }
    }
}

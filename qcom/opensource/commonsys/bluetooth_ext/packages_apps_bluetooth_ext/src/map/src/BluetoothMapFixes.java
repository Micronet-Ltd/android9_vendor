/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.android.bluetooth.map;

import android.bluetooth.BluetoothMap;
import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.util.SparseArray;

import com.android.bluetooth.map.BluetoothMapUtils.TYPE;

public class BluetoothMapFixes {

    private static final String TAG = "BluetoothMapFixes";

    public static final boolean DEBUG = BluetoothMapService.DEBUG;

    public static final boolean VERBOSE = BluetoothMapService.VERBOSE;

    /*to trigger MNS OFF to handle MCE that dosen't issue MNS OFF before MAS DISC*/
    public static void handleMnsShutdown(SparseArray<BluetoothMapMasInstance>
            mMasInstances, int masId) {
        BluetoothMapMasInstance masInst = mMasInstances.get(masId);
        if (masInst != null && masInst.mObserver != null) {
            try {
                masInst.mObserver.setNotificationRegistration(BluetoothMapAppParams
                    .NOTIFICATION_STATUS_NO);
            } catch (RemoteException e) {
                Log.e(TAG,"setNoficationRegistarion OFF Failed: "+ e);
            }
        }
    }

    /*
     * Checks if its the last line of the BMessage
     */
    public static boolean isLastLine(byte [] line) {
        if (line == null || line.length == 0)
            return true;
        return false;
    }

    /**
      * Create both MAP SMS/MMS and EMAIL SDP in a handler thread.
      */
    public static void sendCreateMasInstances(BluetoothMapService mapService,
            int sendCreateMsg) {
        Handler mSessionStatusHandler = mapService.getHandler();
        if (mSessionStatusHandler != null && !mSessionStatusHandler
                .hasMessages(sendCreateMsg)) {
            Log.d(TAG, "mSessionStatusHandler CREATE_MAS_INSTANCES ");
            Message msg = mSessionStatusHandler.obtainMessage(sendCreateMsg);
            /* We add a small delay here to ensure the call returns true before this message is
             * handled. It seems wrong to add a delay, but the alternative is to build a lock
             * system to handle synchronization, which isn't nice either... */
            mSessionStatusHandler.sendMessage(msg);
        } else if(mSessionStatusHandler != null ) {
            Log.w(TAG, "mSessionStatusHandler START_MAPEMAIL message already in Queue");
        }
    }

    /* Check if Map App Observer is null. */
    public static boolean checkMapAppObserver(
            BluetoothMapAppObserver mAppObserver) {
        if (mAppObserver == null) {
            Log.w( TAG, "updateMasInstancesHandler: NoAppObeserver Found");
            return true;
        }
        return false;
    }

    /* to create app observers and get enabled accounts to create MAS instances */
    public static void createMasInstances(BluetoothMapService mService) {
        mService.mAppObserver = new BluetoothMapAppObserver(mService,
                (BluetoothMapService) mService);
        if (mService.mAppObserver != null ) {
            mService.mEnabledAccounts = mService.mAppObserver
                    .getEnabledAccountItems();
        }
       /** Uses mEnabledAccounts, hence getEnabledAccountItems()
          * must be called before this. */
        mService.createMasInstances();
    }

}

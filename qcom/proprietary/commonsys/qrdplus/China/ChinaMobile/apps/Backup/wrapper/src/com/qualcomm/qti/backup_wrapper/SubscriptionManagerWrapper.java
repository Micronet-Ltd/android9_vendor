/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.telephony.SubscriptionManager;
import android.util.Log;

import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class SubscriptionManagerWrapper {

    private static final String TAG = "SubscriptionManagerWrapper";

    public static int getSlotId(int subId) throws WrapperNotSupportException {
        try {
            return SubscriptionManager.getSlotIndex(subId);
        } catch (Exception e) {
            Log.e(TAG,"getSlotId: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }

    public static int[] getSubId(int slotId) throws WrapperNotSupportException {
        try {
            return SubscriptionManager.getSubId(slotId);
        } catch (Exception e) {
            Log.e(TAG,"getSubId: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }
}

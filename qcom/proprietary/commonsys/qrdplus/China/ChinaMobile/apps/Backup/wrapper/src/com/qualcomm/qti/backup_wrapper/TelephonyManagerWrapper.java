/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.telephony.TelephonyManager;
import android.util.Log;

import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class TelephonyManagerWrapper {

    private static final String TAG = "TelephonyManagerWrapper";

    private TelephonyManager mTelephonyManager ;

    private TelephonyManagerWrapper(TelephonyManager instance) {	
        mTelephonyManager = instance;	
    }

    /* @deprecated - use getSystemService as described above */
    public static TelephonyManagerWrapper getDefault() throws WrapperNotSupportException {
        try {
            return new TelephonyManagerWrapper(TelephonyManager.getDefault());
        } catch (Exception e) {
            Log.e(TAG,"getDefault: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }

    public boolean isMultiSimEnabled() throws WrapperNotSupportException {
        try{
            if(mTelephonyManager != null){
                return mTelephonyManager.isMultiSimEnabled();
            }else{
                throw new WrapperNotSupportException(TAG + " instance is null!");
            }
        } catch (Exception e) {
            Log.e(TAG,"isMultiSimEnabled: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }

    public int getPhoneCount() throws WrapperNotSupportException {
        try{
            if(mTelephonyManager != null){
                return mTelephonyManager.getPhoneCount();
            }else{
                throw new WrapperNotSupportException(TAG + " instance is null!");
            }
        } catch (Exception e) {
            Log.e(TAG,"isMultiSimEnabled: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }
}
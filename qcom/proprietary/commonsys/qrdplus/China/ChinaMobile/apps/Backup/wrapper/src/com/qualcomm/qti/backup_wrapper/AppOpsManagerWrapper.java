/*
 *  Copyright (c) 2018 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.app.AppOpsManager;
import android.util.Log;
import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class AppOpsManagerWrapper {

    private static final String TAG = "AppOpsManagerWrapper";
    private AppOpsManager mAppOpsManager;

    public static final int OP_WRITE_SMS = AppOpsManager.OP_WRITE_SMS;

    public AppOpsManagerWrapper(AppOpsManager instance) {
        mAppOpsManager = instance;
    }

    public int checkOp(int op, int uid, String packageName) throws WrapperNotSupportException {
        try{
            if(mAppOpsManager != null){
                return mAppOpsManager.checkOp(op, uid, packageName);
            }else{
                throw new WrapperNotSupportException(TAG + " instance is null!");
            }
        } catch (Exception e) {
            Log.e(TAG,"checkOp: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }

    public void setMode(int code, int uid, String packageName, int mode) throws WrapperNotSupportException {
        try{
            if(mAppOpsManager != null){
                mAppOpsManager.setMode(code, uid, packageName, mode);
            }else{
                throw new WrapperNotSupportException(TAG + " instance is null!");
            }
        } catch (Exception e) {
            Log.e(TAG,"setMode: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }
}
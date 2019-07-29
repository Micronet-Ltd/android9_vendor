/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.annotation.Nullable;
import android.content.IContentProvider;
import android.os.Bundle;
import android.os.RemoteException;
import android.util.Log;
import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class IContentProviderWrapper {

    private static final String TAG = "IContentProviderWrapper";
    private IContentProvider mIContentProvider = null;

    public IContentProviderWrapper(IContentProvider instance){
        mIContentProvider = instance;
    }

    public void setInstance(IContentProvider instance){
        mIContentProvider = instance;
    }

    public IContentProvider getInstance() {
        return mIContentProvider;
    }

    public  Bundle call(
            String callingPkg, String method, @Nullable String arg, @Nullable Bundle extras)
            throws RemoteException {
                try {
                    return mIContentProvider.call(callingPkg, method, arg, extras);
                } catch (RemoteException e) {
                    throw e;
                }
            }

}

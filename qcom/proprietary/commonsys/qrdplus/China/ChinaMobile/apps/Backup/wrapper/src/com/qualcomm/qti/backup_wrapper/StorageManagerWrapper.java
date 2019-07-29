/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.annotation.NonNull;
import android.content.Context;
import android.os.storage.StorageManager;

import com.qualcomm.qti.backup_wrapper.StorageVolumeWrapper;
import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class StorageManagerWrapper {
    
    private static final String TAG = "StorageManagerWrapper";
    private StorageManager sm;

    public StorageManagerWrapper(StorageManager instance) {
        sm = instance;
    }

    public static StorageManager from(Context context) throws WrapperNotSupportException {
        try {
            return StorageManager.from(context);
        } catch (Exception e) {
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }

    public @NonNull StorageVolumeWrapper[] getVolumeList() throws WrapperNotSupportException {
        try {
            if(sm != null) {
                return StorageVolumeWrapper.getVolumeWrapperList(sm.getVolumeList());
            } else {
                throw new WrapperNotSupportException(TAG + " instance is null!");
            }
        } catch (Exception e) {
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }

}
/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.os.storage.StorageVolume;

import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class StorageVolumeWrapper {

    private static final String TAG = "StorageVolumeWrapper";

    private StorageVolume mStorageVolume;

    public StorageVolumeWrapper(StorageVolume instance) {
        mStorageVolume = instance;
    }

    public static StorageVolumeWrapper[] getVolumeWrapperList(StorageVolume[] valumes) {
        StorageVolumeWrapper[] mValumes;
        if(valumes != null) {
            mValumes = new StorageVolumeWrapper[valumes.length];
            for(int i = 0; i < valumes.length; i++) {
                mValumes[i] = new StorageVolumeWrapper(valumes[i]);
            }
            return mValumes;
        } else {
            return null;
        }
    }

    public boolean isRemovable() throws WrapperNotSupportException {
        try {
            return mStorageVolume.isRemovable();
        } catch (Exception e) {
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }

    public boolean allowMassStorage() throws WrapperNotSupportException {
        try {
            return mStorageVolume.allowMassStorage();
        } catch (Exception e) {
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }

    public String getPath() throws WrapperNotSupportException {
        try {
            return mStorageVolume.getPath();
        } catch (Exception e) {
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }
}
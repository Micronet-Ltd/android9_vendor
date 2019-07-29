/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.os.storage.StorageEventListener;
import android.os.storage.StorageManager;
import android.util.Log;

import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public abstract class StorageEventListenerWrapper{

    private static final String TAG = "StorageEventListenerWrapper";

    private StorageEventListener mStorageListener;
    
    public StorageEventListenerWrapper(){
        mStorageListener = new StorageEventListener() {
            @Override
            public void onStorageStateChanged(String path, String oldState, String newState) {
                Log.d(TAG, "onStorageStateChanged");
                StorageEventListenerWrapper.this.onStorageStateChanged(path, oldState, newState);
            }
        };
    }

    public void unregisterListener(StorageManager storageManager) throws WrapperNotSupportException {
        if(storageManager == null) {
            throw new WrapperNotSupportException(TAG + "storageManager is null!");
        }
        try {
            if(mStorageListener != null){
                Log.d(TAG, "unregisterListener");
                storageManager.unregisterListener(mStorageListener);
            }
        } catch (Exception e) {
            Log.e(TAG,"unregisterListener: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }
    
    public void registerListener(StorageManager storageManager) throws WrapperNotSupportException {
        if(storageManager == null) {
            throw new WrapperNotSupportException(TAG + "storageManager is null!");
        }
        try {
            if(mStorageListener != null){
                Log.d(TAG, "registerListener");
                storageManager.registerListener(mStorageListener);
            }
        } catch (Exception e) {
            Log.e(TAG,"unregisterListener: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
        
    }
    
    public abstract void onStorageStateChanged(String path, String oldState, String newState);

}
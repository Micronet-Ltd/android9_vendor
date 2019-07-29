/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.smartassistant;

import android.app.Application;

import com.qualcomm.qti.smartassistant.sva.controller.ExtendedSmMgr;
import com.qualcomm.qti.smartassistant.sva.controller.AssetsFileMgr;

public class SmartAssistantApplication extends Application {
    private static SmartAssistantApplication sInstance;
    private ExtendedSmMgr mExtendedSmMgr;
    private AssetsFileMgr mAssetsFileMgr;

    public static SmartAssistantApplication getInstance() {
        return sInstance;
    }

    @Override
    public void onCreate() {
        sInstance = this;
        super.onCreate();
        mExtendedSmMgr = new ExtendedSmMgr();
        mAssetsFileMgr = new AssetsFileMgr();
    }

    public ExtendedSmMgr getExtendedSmMgr(){
        return mExtendedSmMgr;
    }

    public AssetsFileMgr getAssetsFileMgr(){
        return mAssetsFileMgr;
    }
}
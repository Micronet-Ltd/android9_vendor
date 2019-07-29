/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.smartassistant.asr.baidu;

import android.os.Handler;

import com.baidu.duer.dcs.systeminterface.BaseAudioRecorder;

public abstract class AbstractDuerAudioRecorder  extends BaseAudioRecorder{

    private IAsrRecordingListener mAsrRecordingListener;
    private Handler mHandler = new Handler();

    public interface IAsrRecordingListener{

        void onAsrRecordingStarted();

        void onAsrRecordingStopped();
    }

    public void setAsrRecordingListener(IAsrRecordingListener listener){
        mAsrRecordingListener = listener;
    }

    void notifyAsrRecordingStarted(){
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mAsrRecordingListener != null){
                    mAsrRecordingListener.onAsrRecordingStarted();
                }
            }
        });
    }

    void notifyAsrRecordingStopped(){
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mAsrRecordingListener != null){
                    mAsrRecordingListener.onAsrRecordingStopped();
                }
            }
        });
    }
}

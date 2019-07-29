/*
* Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.smartassistant.asr.baidu;

import com.qualcomm.qti.smartassistant.recordings.IRecordingListener;
import com.qualcomm.qti.smartassistant.recordings.RecordingMgr;
import com.qualcomm.qti.smartassistant.utils.LogUtils;

public class DuerAudioRecordImpl extends AbstractDuerAudioRecorder {
    private static final String TAG = DuerAudioRecordImpl.class.getSimpleName();
    private final RecordingMgr mRecordingMgr;
    private Thread mAsrThread;
    private volatile boolean isStartRecord = false;

    private IRecordingListener listener = new IRecordingListener() {
        @Override
        public void onRecordingStarted() {
            //do nothing
        }

        @Override
        public void onAudioData(byte[] buffer) {
            //do nothing
        }

        @Override
        public void onRecordingStopped() {
            notifyAsrRecordingStopped();
        }
    };

    public DuerAudioRecordImpl(RecordingMgr recordingMgr) {
        mRecordingMgr = recordingMgr;
        mRecordingMgr.addRecordingListener(listener);
    }

    @Override
    public void startRecord() {
        LogUtils.v(TAG, "startRecord ");
        this.isStartRecord = true;
        notifyAsrRecordingStarted();
        if (!mRecordingMgr.isRecordingStarted()){
            mRecordingMgr.startRecord(null);
        }
        mAsrThread = new Thread() {
            public void run() {
                super.run();
                while(isStartRecord) {
                    try {
                        byte[] buffer = mRecordingMgr.readAudioData();
                        if (buffer != null){
                            fireData(buffer);
                        }
                    } catch (Exception var3) {
                        isStartRecord = false;
                    }
                }
            }
        };
        mAsrThread.start();
    }

    @Override
    public void stopRecord() {
        LogUtils.v(TAG, "stopRecord ");
        this.isStartRecord = false;
        if(mAsrThread != null) {
            mAsrThread.interrupt();
            mAsrThread = null;
        }
        mRecordingMgr.stopRecord();
    }
}

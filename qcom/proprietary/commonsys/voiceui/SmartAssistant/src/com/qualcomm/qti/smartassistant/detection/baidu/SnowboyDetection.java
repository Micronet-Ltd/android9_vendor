/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.smartassistant.detection.baidu;

import android.content.Context;
import android.os.Environment;
import android.os.Handler;

import com.qualcomm.qti.smartassistant.recordings.IRecordingListener;
import com.qualcomm.qti.smartassistant.detection.ISecondStageDetection;
import com.qualcomm.qti.smartassistant.recordings.RecordingMgr;
import com.qualcomm.qti.smartassistant.sva.session.IRecognitionEvent;
import com.qualcomm.qti.smartassistant.utils.FileUtils;
import com.qualcomm.qti.smartassistant.utils.LogUtils;
import com.qualcomm.qti.smartassistant.utils.SharedPreferenceUtils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

import ai.kitt.snowboy.SnowboyDetect;

public class SnowboyDetection implements ISecondStageDetection {
    private static final String TAG = SnowboyDetection.class.getSimpleName();
    private static final String DEFAULT_WORK_SPACE = Environment.
            getExternalStorageDirectory().getAbsolutePath() + "/SmartAssistant/";
    private static final String ACTIVE_UMDL = DEFAULT_WORK_SPACE +
            "xiaoduxiaodu_all_11272017.umdl";
    private static final String ACTIVE_RES = DEFAULT_WORK_SPACE + "common.res";
    private final SnowboyDetect mDetector;
    private final RecordingMgr mRecordingMgr;
    private final Context mContext;
    private final Handler mHandler = new Handler();
    private List<IDetectionListener> mDetectionListeners = new ArrayList<>();
    private long mMaxDetectionByteSize = 0;
    private long mShortsRead = 0;

    private IRecordingListener listener = new IRecordingListener() {

        @Override
        public void onRecordingStarted() {
            //do nothing
        }

        @Override
        public void onAudioData(byte[] buffer) {
            runSnowboyDetection(buffer);
        }

        @Override
        public void onRecordingStopped() {
            mRecordingMgr.removeRecordingListener(this);
        }
    };

    public SnowboyDetection(Context context, RecordingMgr recordingMgr){
        mContext = context;
        mDetector = new SnowboyDetect(ACTIVE_RES, ACTIVE_UMDL);
        mRecordingMgr = recordingMgr;
        mDetector.SetSensitivity("0.35,0.35,0.40");
        mDetector.SetHighSensitivity("0.45,0.45,0.55");
        mDetector.ApplyFrontend(true);
        mMaxDetectionByteSize = FileUtils.getHistBufferTimeSetting() / 1000
                * RecordingMgr.SAMPLE_RATE_HZ * 2;
        LogUtils.d(TAG,"mMaxDetectionByteSize = "+mMaxDetectionByteSize);
    }

    @Override
    public void addSecondStageDetectionListener(IDetectionListener listener) {
        if(!mDetectionListeners.contains(listener)){
            mDetectionListeners.add(listener);
        }
    }

    @Override
    public void removeSecondStageDetectionListener(IDetectionListener listener) {
        if (mDetectionListeners.contains(listener)){
            mDetectionListeners.remove(listener);
        }
    }

    @Override
    public boolean isEnabled() {
        return SharedPreferenceUtils.getIsSecondDetectionEnabled(mContext);
    }

    @Override
    public void startSecondDetection(IRecognitionEvent recognitionEvent){
        resetDetection();
        mRecordingMgr.addRecordingListener(listener);
    }

    @Override
    public void stopSecondDetection(){
        mRecordingMgr.removeRecordingListener(listener);
    }

    private void runSnowboyDetection(byte[] audioBuffer){
        if (mShortsRead == 0){
            LogUtils.e(TAG,"runSnowboyDetection begin");
        }
        // Converts to short array.
        short[] audioData = new short[audioBuffer.length / 2];
        ByteBuffer.wrap(audioBuffer).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().get(audioData);

        mShortsRead += audioData.length;

        // Snowboy hotword detection.
        int result = mDetector.RunDetection(audioData, audioData.length);

        /**
         * -2 VAD_NO_SPEECH
         * -1 VAD_ERROR
         * =0 VAD_NORNAL_SPEECH
         * >0 Detected
         */
        if (result > 0){
            LogUtils.d(TAG, "Hotword " + Integer.toString(result)
                    + " detected!" + " shortsRead = "+mShortsRead);
            notifyKeywordDetected();
            mDetector.Reset();
            mShortsRead = 0;
        }else {
            if ((mShortsRead * 2) >= mMaxDetectionByteSize){
                mDetector.Reset();
                notifyKeywordUndetected();
                mShortsRead = 0;
            }
        }
    }

    private void resetDetection(){
        mDetector.Reset();
        mShortsRead = 0;
    }

    private void notifyKeywordDetected(){
        mHandler.removeCallbacks(mNotifyHotwordDetectedTask);
        mHandler.post(mNotifyHotwordDetectedTask);
        mRecordingMgr.removeRecordingListener(listener);
        SharedPreferenceUtils.increaseSecondDetectionCount();
    }

    private void notifyKeywordUndetected(){
        LogUtils.d(TAG,"notifyKeywordUndetected ");
        mHandler.removeCallbacks(mNotifyHotwordUndetectedTask);
        mHandler.post(mNotifyHotwordUndetectedTask);
        mRecordingMgr.removeRecordingListener(listener);
    }

    private final Runnable mNotifyHotwordDetectedTask = new Runnable() {
        @Override
        public void run() {
            for (IDetectionListener listener: mDetectionListeners){
                listener.onHotwordDetected();
            }
        }
    };

    private final Runnable mNotifyHotwordUndetectedTask = new Runnable() {
        @Override
        public void run() {
            for (IDetectionListener listener: mDetectionListeners){
                listener.onHotwordUndetected();
            }
        }
    };
}

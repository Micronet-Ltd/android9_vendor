/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.smartassistant.recordings;

import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.os.Handler;
import android.os.Process;
import android.os.RemoteException;
import android.os.IDeviceIdleController;
import android.os.ServiceManager;

import com.qualcomm.qti.smartassistant.SmartAssistantApplication;
import com.qualcomm.qti.smartassistant.sva.session.IRecognitionEvent;
import com.qualcomm.qti.smartassistant.utils.FileUtils;
import com.qualcomm.qti.smartassistant.utils.LogUtils;
import com.qualcomm.qti.smartassistant.wrapper.AudioRecordWrapper;

import java.nio.ByteBuffer;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.LinkedBlockingQueue;

public class RecordingMgr {

    private static final String TAG = RecordingMgr.class.getSimpleName();
    private final static int BUFFER_TIMER_INTERVAL = 2000;
    private final static int READ_TIMER_INTERVAL = 120;
    private final static int BITS_PER_SAMPLE = 16;
    private final static int AUDIO_SOURCE = 1;
    public static final int SAMPLE_RATE_HZ = 16000;
    private static final int POWER_SAVE_TEMP_WHITELIST_ENABLED_DURATION = 1000 * 10;
    private static final int MAX_SAVED_AUDIO_RECORDING_DURATION = 300;
    private static final int MAX_AUDIO_RECORDING_BYTES =
            MAX_SAVED_AUDIO_RECORDING_DURATION * SAMPLE_RATE_HZ * 2;
    private int mAudioRecordBufferSize;
    private int mReadBufferSize;
    private List<IRecordingListener> mRecordingListeners = Collections.
            synchronizedList(new CopyOnWriteArrayList<IRecordingListener>());
    private Handler mHandler = new Handler();
    private volatile boolean mIsStartRecord = false;
    private int mTotalBytesToSave;
    private Thread mRecordThread;
    private long mKeywordEndIndex;

    private LinkedBlockingQueue<AudioDataNode> mAudioDataQueue = new LinkedBlockingQueue<>();

    public RecordingMgr() {
        int bufferSize = AudioRecord.getMinBufferSize(SAMPLE_RATE_HZ,
                AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);

        int bufferFramePeriod = SAMPLE_RATE_HZ * BUFFER_TIMER_INTERVAL / 1000;
        mAudioRecordBufferSize = bufferFramePeriod * BITS_PER_SAMPLE * AUDIO_SOURCE / 8;

        int readFramePeriod = SAMPLE_RATE_HZ * READ_TIMER_INTERVAL / 1000;
        mReadBufferSize = readFramePeriod * BITS_PER_SAMPLE * AUDIO_SOURCE / 8;

        if (mAudioRecordBufferSize < bufferSize) {
            mAudioRecordBufferSize = bufferSize;
            mReadBufferSize = bufferSize;
        }

        LogUtils.d(TAG, "mAudioRecordBufferSize = " + mAudioRecordBufferSize
                + "; mReadBufferSize = " + mReadBufferSize + "; minBufferSize = " + bufferSize);
    }

    public void startRecord(final IRecognitionEvent recognitionEvent){
        notifyRecordingStarted();
        mAudioDataQueue.clear();
        mIsStartRecord = true;
        mRecordThread = new Thread() {
            public void run() {
                super.run();
                addPowerSaveTempWhitelistApp();
                Process.setThreadPriority(Process.THREAD_PRIORITY_AUDIO);
                AudioRecord audioRecord = createAudioRecorder(recognitionEvent);

                if (audioRecord.getState() != AudioRecord.STATE_INITIALIZED){
                    LogUtils.e(TAG, "audio record initialization check failed");
                    return;
                }
                LogUtils.v(TAG, "startRecording ");
                try {
                    audioRecord.startRecording();
                } catch (IllegalStateException var4) {
                    var4.printStackTrace();
                    LogUtils.e(TAG, "startRecording IllegalStateException ", var4);
                }
                ByteBuffer totalBuffer = ByteBuffer.allocate(mTotalBytesToSave);
                int curBytesRead = 0;
                while(mIsStartRecord) {
                    try {
                        byte[] buffer = new byte[mReadBufferSize];
                        int readBytes = audioRecord.read(buffer, 0, mReadBufferSize);
                        if(readBytes > 0) {
                            mAudioDataQueue.put(new AudioDataNode(buffer,curBytesRead));
                            fireAudioData(buffer);
                            if (totalBuffer != null && (curBytesRead + readBytes) < mTotalBytesToSave){
                                totalBuffer.put(buffer,0, readBytes);
                                curBytesRead += readBytes;
                            }
                        }
                    } catch (Exception var3) {
                        mIsStartRecord = false;
                    }
                }

                audioRecord.stop();
                audioRecord.release();
                notifyRecordingStopped();
                LogUtils.v(TAG, "audioRecorder released ");

                if (curBytesRead > 0 && FileUtils.getIsSavePcmDataSetting()) {
                    LogUtils.v(TAG, "curBytesRead = " + curBytesRead);
                    String directory = recognitionEvent != null
                            && recognitionEvent.getCaptureAvailable() ?
                            FileUtils.LOOK_AHEAD_VOICE_COMMANDS_PATH :
                            FileUtils.NORMAL_VOICE_COMMANDS_PATH;
                    String filePath = directory + "/"+ String.valueOf(System.currentTimeMillis())+
                            FileUtils.PATH_RECORDINGS_FILE_EXT;
                    FileUtils.writeBufferToWavFile(totalBuffer.array(),curBytesRead,filePath,
                            false);
                }
            }
        };
        mRecordThread.start();
    }

    public void stopRecord(){
        LogUtils.v(TAG, "stopRecord ");
        mIsStartRecord = false;
        if(mRecordThread != null) {
            mRecordThread.interrupt();
            mRecordThread = null;
        }
    }

    public boolean isRecordingStarted(){
        return mIsStartRecord;
    }

    public void addRecordingListener(IRecordingListener listener){
        if (!mRecordingListeners.contains(listener)){
            mRecordingListeners.add(listener);
        }
    }

    public void removeRecordingListener(IRecordingListener listener){
        if (mRecordingListeners.contains(listener)){
            mRecordingListeners.remove(listener);
        }
    }

    private void notifyRecordingStarted(){
        mHandler.removeCallbacks(mNotifyRecordingStartedTask);
        mHandler.post(mNotifyRecordingStartedTask);
    }

    private void notifyRecordingStopped(){
        mHandler.removeCallbacks(mNotifyRecordingStoppedTask);
        mHandler.post(mNotifyRecordingStoppedTask);
    }

    private void fireAudioData(byte[] audioData){
        for (IRecordingListener listener: mRecordingListeners){
            listener.onAudioData(audioData);
        }
    }

    public byte[] readAudioData(){
        try {
            AudioDataNode node = mAudioDataQueue.take();
            if (node.getStartIndex() >= mKeywordEndIndex){
                return node.getAudioData();
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        return null;
    }

    private void addPowerSaveTempWhitelistApp(){
        IDeviceIdleController deviceIdleController = getIDeviceIdleController();
        if(deviceIdleController != null){
            try {
                deviceIdleController.addPowerSaveTempWhitelistApp(
                        SmartAssistantApplication.getInstance().getPackageName(),
                        POWER_SAVE_TEMP_WHITELIST_ENABLED_DURATION, 0, "SmartAssistant");
            }catch (RemoteException e){
                e.printStackTrace();
            }
        }
    }

    private IDeviceIdleController getIDeviceIdleController(){
        return IDeviceIdleController.Stub.asInterface(
                ServiceManager.getService(Context.DEVICE_IDLE_CONTROLLER));
    }

    private AudioRecord createAudioRecorder(IRecognitionEvent recognitionEvent){
        if (recognitionEvent != null && recognitionEvent.getCaptureAvailable()){
            return createOneShotAudioRecord(recognitionEvent);
        }else {
            return createNormalAudioRecord();
        }
    }

    private AudioRecord createNormalAudioRecord(){
        mKeywordEndIndex = 0;
        mTotalBytesToSave = MAX_AUDIO_RECORDING_BYTES;
        return new AudioRecord(1, SAMPLE_RATE_HZ,
                AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT,
                mAudioRecordBufferSize);
    }

    private AudioRecord createOneShotAudioRecord(IRecognitionEvent recognitionEvent){
        final int configOneShotBufferTime = FileUtils.getOneShotBufferTimeSetting() / 1000;
        final int oneShotBufferTime = configOneShotBufferTime >
                MAX_SAVED_AUDIO_RECORDING_DURATION ? MAX_SAVED_AUDIO_RECORDING_DURATION :
                configOneShotBufferTime;
        mTotalBytesToSave = oneShotBufferTime * SAMPLE_RATE_HZ * 2;
        mKeywordEndIndex = recognitionEvent.getEndIndex();
        /** if device or model not support, crop history buffer*/
        if (mKeywordEndIndex == 0){
            mKeywordEndIndex = (FileUtils.getHistBufferTimeSetting()/1000) * SAMPLE_RATE_HZ * 2;
        }
        return AudioRecordWrapper.createOneShotAudioRecord(mAudioRecordBufferSize,
                recognitionEvent.getCaptureSession());
    }

    private final Runnable mNotifyRecordingStartedTask = new Runnable() {
        @Override
        public void run() {
            for (IRecordingListener listener: mRecordingListeners){
                listener.onRecordingStarted();
            }
        }
    };

    private final Runnable mNotifyRecordingStoppedTask = new Runnable() {
        @Override
        public void run() {
            for (IRecordingListener listener: mRecordingListeners){
                listener.onRecordingStopped();
            }
        }
    };
}

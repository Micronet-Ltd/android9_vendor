/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.smartassistant.sva.session;

import android.hardware.soundtrigger.SoundTrigger;

import com.qualcomm.qti.smartassistant.sva.opaque.OpaqueDataDecoder;


final public class RecognitionEvent implements IRecognitionEvent {
    private final static String TAG = RecognitionEvent.class.getSimpleName();

    private final int mStatus;
    private final int mSoundModelHandle;
    private final boolean mCaptureAvailable;
    private final int mCaptureSession;
    private final long mBeginIndex;
    private final long mEndIndex;


    public RecognitionEvent(SoundTrigger.RecognitionEvent recognitionEvent) {
        mStatus = recognitionEvent.status;
        mSoundModelHandle = recognitionEvent.soundModelHandle;
        mCaptureAvailable = recognitionEvent.captureAvailable;
        mCaptureSession = recognitionEvent.captureSession;
        OpaqueDataDecoder opaqueDataDecoder = new OpaqueDataDecoder(recognitionEvent.data);
        mBeginIndex = opaqueDataDecoder.getBeginIndex();
        mEndIndex = opaqueDataDecoder.getEndIndex();
    }

    @Override
    public State getState() {
        State state = State.FAILURE;

        switch (mStatus) {
            case SoundTrigger.RECOGNITION_STATUS_SUCCESS:
                state = State.SUCCESS;
                break;
            case SoundTrigger.RECOGNITION_STATUS_ABORT:
                state = State.ABORT;
                break;
            case SoundTrigger.RECOGNITION_STATUS_FAILURE:
                state = State.FAILURE;
                break;
        }

        return state;
    }

    @Override
    public int getSoundModelHandle() {
        return mSoundModelHandle;
    }

    @Override
    public boolean getCaptureAvailable() {
        return mCaptureAvailable;
    }

    @Override
    public int getCaptureSession() {
        return mCaptureSession;
    }

    @Override
    public long getBeginIndex() {
        return mBeginIndex;
    }

    @Override
    public long getEndIndex() {
        return mEndIndex;
    }

    @Override
    public String toString() {
        return "RecognitionEvent{" +
                "mStatus=" + mStatus +
                ", mSoundModelHandle=" + mSoundModelHandle +
                ", mCaptureAvailable=" + mCaptureAvailable +
                ", mCaptureSession=" + mCaptureSession +
                ", mBeginIndex=" + mBeginIndex +
                ", mEndIndex=" + mEndIndex +
                '}';
    }

}

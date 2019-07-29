/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.smartassistant.sva.data;

import android.hardware.soundtrigger.SoundTrigger;

import com.qualcomm.qti.smartassistant.utils.LogUtils;

public class ExtendedSmModel extends SmModel implements IExtendedSmModel {
    private final String TAG = ExtendedSmModel.class.getSimpleName();

    private SessionStatus mSessionStatus = SessionStatus.UNLOADED;
    private SoundTrigger.KeyphraseRecognitionExtra[] mKpRecognitionExtraArray;
    private SoundTrigger.RecognitionConfig mRecognitionConfig;

    public ExtendedSmModel(String smFullFileName) {
        super(smFullFileName);
    }

    @Override
    public SessionStatus getSessionStatus() {
        return mSessionStatus;
    }

    @Override
    public void setSessionStatus(SessionStatus status) {
        mSessionStatus = status;
    }

    @Override
    public SoundTrigger.KeyphraseRecognitionExtra[] getKeyphraseRecognitionExtra() {
        return mKpRecognitionExtraArray;
    }

    @Override
    public void setKeyphraseRecognitionExtra(SoundTrigger.KeyphraseRecognitionExtra[] extraArray) {
        mKpRecognitionExtraArray = extraArray.clone();
    }

    @Override
    public SoundTrigger.RecognitionConfig getRecognitionConfig() {
        return mRecognitionConfig;
    }

    @Override
    public void setRecognitionConfig(SoundTrigger.RecognitionConfig config) {
        mRecognitionConfig = config;
    }
}

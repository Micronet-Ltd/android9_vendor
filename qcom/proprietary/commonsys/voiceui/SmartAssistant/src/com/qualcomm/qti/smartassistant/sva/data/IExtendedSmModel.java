/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.smartassistant.sva.data;

import android.hardware.soundtrigger.SoundTrigger;

public interface IExtendedSmModel extends ISmModel {
    SessionStatus getSessionStatus();

    void setSessionStatus(SessionStatus status);

    SoundTrigger.KeyphraseRecognitionExtra[] getKeyphraseRecognitionExtra();

    void setKeyphraseRecognitionExtra(SoundTrigger.KeyphraseRecognitionExtra[] extraArray);

    SoundTrigger.RecognitionConfig getRecognitionConfig();

    void setRecognitionConfig(SoundTrigger.RecognitionConfig config);

    enum SessionStatus {
        UNLOADED,
        LOADED,
        STARTED,
        STOPPED
    }

}

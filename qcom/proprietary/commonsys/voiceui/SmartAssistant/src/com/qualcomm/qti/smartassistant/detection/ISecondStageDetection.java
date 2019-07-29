/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.smartassistant.detection;

import com.qualcomm.qti.smartassistant.sva.session.IRecognitionEvent;

public interface ISecondStageDetection {

    void startSecondDetection(IRecognitionEvent recognitionEvent);

    void stopSecondDetection();

    void addSecondStageDetectionListener(IDetectionListener listener);

    void removeSecondStageDetectionListener(IDetectionListener listener);

    boolean isEnabled();

    interface IDetectionListener{
        void onHotwordDetected();

        void onHotwordUndetected();

    }
}

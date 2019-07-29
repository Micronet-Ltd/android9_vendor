/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.smartassistant.sva.session;


public interface IRecognitionEvent {
    enum State {
        SUCCESS, ABORT, FAILURE;

        public boolean isSuccess() {
            return this == SUCCESS;
        }
    }

    State getState();
    int getSoundModelHandle();
    boolean getCaptureAvailable();
    int getCaptureSession();
    long getBeginIndex();
    long getEndIndex();
}

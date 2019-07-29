/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.smartassistant.recordings;

public interface IRecordingListener {

    void onRecordingStarted();

    void onAudioData(byte[] buffer);

    void onRecordingStopped();
}

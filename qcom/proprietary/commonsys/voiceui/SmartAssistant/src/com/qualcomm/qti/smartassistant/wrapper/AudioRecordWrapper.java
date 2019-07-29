/*
* Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.smartassistant.wrapper;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioRecord;

public final class AudioRecordWrapper {

    public static AudioRecord createOneShotAudioRecord(int bufferSize, int sessionNum) {
        return new AudioRecord(
                createAudioAttributes(),
                createAudioFormat(),
                bufferSize,
                sessionNum
        );
    }

    private static AudioAttributes createAudioAttributes() {
        return new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_MEDIA)
                .setContentType(AudioAttributes.CONTENT_TYPE_SPEECH)
                .setInternalCapturePreset(1999)
                .build();
    }

    private static AudioFormat createAudioFormat() {
        return new AudioFormat.Builder()
                .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                .setSampleRate(16000)
                .setChannelMask(AudioFormat.CHANNEL_IN_MONO)
                .build();
    }
}

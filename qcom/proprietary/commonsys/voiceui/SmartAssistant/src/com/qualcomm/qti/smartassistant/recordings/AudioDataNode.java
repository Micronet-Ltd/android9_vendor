/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.smartassistant.recordings;

public class AudioDataNode {

    private byte[] mAudioData;
    private int mStartIndex;

    public AudioDataNode(byte[] data, int startIndex){
        mAudioData = data;
        mStartIndex = startIndex;
    }

    public byte[] getAudioData(){
        return mAudioData;
    }

    public int getStartIndex(){
        return mStartIndex;
    }
}

/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.smartassistant.sva.opaque;

import com.qualcomm.qti.smartassistant.sva.opaque.OpaqueDataEncoder.ST_Param_Key;
import com.qualcomm.qti.smartassistant.utils.LogUtils;


public class OpaqueDataDecoder {
    private final static String TAG = OpaqueDataDecoder.class.getSimpleName();

    private int mVersion = -1;
    private int mBeginIndex = 0;
    private int mEndIndex = 0;

    public OpaqueDataDecoder(byte[] opaqueData) {
        parse(opaqueData);
    }

    public int getVersion() {
        return mVersion;
    }

    public int getBeginIndex() {
        return mBeginIndex;
    }

    public int getEndIndex() {
        return mEndIndex;
    }

    private void parse(byte[] opaqueData) {
        LogUtils.d(TAG, "parse opaqueData.length = " + opaqueData.length);
        if (opaqueData.length == 1) return;

        int index = 0;
        while (index < opaqueData.length) {
            if (index + 8 >= opaqueData.length) break;

            int tag = byte2Int(opaqueData, index);
            index += 4;
            int payloadSize = byte2Int(opaqueData, index);
            index += 4;

            LogUtils.d(TAG, "tag = " + tag);
            LogUtils.d(TAG, "payloadSize = " + payloadSize);
            if (tag == ST_Param_Key.ST_PARAM_KEY_CONFIDENCE_LEVELS.ordinal()
                    || tag == ST_Param_Key.ST_PARAM_KEY_HISTORY_BUFFER_CONFIG.ordinal()
                    || tag == ST_Param_Key.ST_PARAM_KEY_TIMESTAMP.ordinal()) {
                index += payloadSize;
            } else if (tag == ST_Param_Key.ST_PARAM_KEY_KEYWORD_INDICES.ordinal()) {
                LogUtils.d(TAG, "ST_PARAM_KEY_KEYWORD_INDICES = " + tag);
                if (index + 12 >= opaqueData.length) break;
                mVersion = byte2Int(opaqueData, index);
                index += 4;
                mBeginIndex = byte2Int(opaqueData, index);
                index += 4;
                mEndIndex = byte2Int(opaqueData, index);
                LogUtils.e(TAG,"mBeginIndex = "+mBeginIndex+"  mEndIndex = "+mEndIndex);
                break;
            }
        }
    }

    private static int byte2Int(byte[] bytes, int start) {
        return (bytes[start++] & 0xff)
                | (bytes[start++] & 0xff) << 8
                | (bytes[start++] & 0xff) << 16
                | (bytes[start] & 0xff) << 24;
    }

}

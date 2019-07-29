/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.session;

import android.hardware.soundtrigger.SoundTrigger;

import com.qualcomm.listen.ListenSoundModel;
import com.qualcomm.listen.ListenTypes;
import com.qualcomm.qti.sva.controller.Global;
import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.utils.FileUtils;
import com.qualcomm.qti.sva.utils.LogUtils;

import java.io.IOException;
import java.nio.ByteBuffer;

public class OpaqueDataHelper {
    private final String TAG = OpaqueDataHelper.class.getSimpleName();

    /*
    #define ST_MAX_SOUND_MODELS 10
    #define ST_MAX_CONFIDENCE_KEYWORDS 10
    #define ST_MAX_CONFIDENCE_USERS 10
    */
    private final int ST_MAX_SOUND_MODELS = 10;
    private final int ST_MAX_CONFIDENCE_KEYWORDS = 10;
    private final int ST_MAX_CONFIDENCE_USERS = 10;


    /*
    enum st_sound_model_id {
        ST_SM_ID_NONE = 0x0000,
        ST_SM_ID_SVA_GMM = 0x0001,
        ST_SM_ID_SVA_CNN = 0x0002,
        ST_SM_ID_SVA_VOP = 0x0004,
        ST_SM_ID_SVA_END = 0x0080,
        ST_SM_ID_CUSTOM_START = 0x0100,
        ST_SM_ID_CUSTOM_END = 0x8000,
    };
    */
    private final int SIZE_OF_ENUM_ST_SOUND_MODEL_ID = 4;
    private final int ST_SM_ID_NONE = 0x0000;
    private final int ST_SM_ID_SVA_GMM = 0x0001;
    private final int ST_SM_ID_SVA_CNN = 0x0002;
    private final int ST_SM_ID_SVA_VOP = 0x0004;
    private final int ST_SM_ID_SVA_END = 0x0080;
    private final int ST_SM_ID_CUSTOM_START = 0x0100;
    private final int ST_SM_ID_CUSTOM_END = 0x8000;
    private final int SIZE_OF_ST_PARAM_KEY = 4;
    /*
    // struct size define
    struct st_param_header
    {
        st_param_key_t key_id;
        uint32_t payload_size;
    };
    */
    private final int SIZE_OF_ST_PARAM_HEADER = SIZE_OF_ST_PARAM_KEY + 4;
    /*
    struct st_user_levels
    {
        uint32_t user_id;
        int32_t level;
    };
    */
    private final int SIZE_OF_ST_USER_LEVEL = 8;
    /*
    struct st_keyword_levels
    {
        int32_t kw_level;
        uint32_t num_user_levels;
        struct st_user_levels user_levels[ST_MAX_CONFIDENCE_USERS];
    };
    */
    private final int SIZE_OF_ST_KEYWORD_LEVELS = 8
            + ST_MAX_CONFIDENCE_USERS * SIZE_OF_ST_USER_LEVEL;
    /*
    struct st_sound_model_conf_levels
    {
        st_sound_model_id_t sm_id;
        uint32_t num_kw_levels;
        struct st_keyword_levels kw_levels[ST_MAX_CONFIDENCE_KEYWORDS];
    };
    */
    private final int SIZE_OF_ST_SOUND_MODEL_CONF_LEVELS = SIZE_OF_ENUM_ST_SOUND_MODEL_ID + 4
            + ST_MAX_CONFIDENCE_KEYWORDS * SIZE_OF_ST_KEYWORD_LEVELS;
    /*
    struct st_confidence_levels_info
    {
        uint32_t version;
        uint32_t num_sound_models;
        struct st_sound_model_conf_levels conf_levels[ST_MAX_SOUND_MODELS];
    };
    */
    private final int CONF_LEVELS_INTF_VERSION_0002 = 0x02;
    private final int SIZE_OF_ST_CONFIDENCE_LEVELS_INFO = 8
            + ST_MAX_SOUND_MODELS * SIZE_OF_ST_SOUND_MODEL_CONF_LEVELS;
    /*
    struct st_hist_buffer_info
    {
        uint32_t version;
        uint32_t hist_buffer_duration_msec;
        uint32_t pre_roll_duration_msec;
    };
    */
    private final int SIZE_OF_ST_HIST_BUFFER_INFO = 12;
    /*
    struct st_keyword_indices_info
    {
        uint32_t version;
        uint32_t start_index; // in bytes
        uint32_t end_index;   // in bytes
    };
    */
    private final int SIZE_OF_ST_KEYWORD_INDICES_INFO = 12;
    /*
    public static native long System.currentTimeMillis()
     */
    private final int SIZE_OF_TIME_STAMP_INFO = 8;
    private ListenTypes.SVASoundModelInfo mSmInfo;
    private boolean mHasConfidenceLevelsParam;
    private boolean mHasHistBufferConfigParam;
    private boolean mHasKeywordIndicesParam;
    private boolean mHasTimeStampParam;
    private boolean mHasVOP = true;
    private boolean mHasGMM = true;
    private boolean mHasCNN = true;
    private int mNumOfModels = 3;
    private String mSmName;

    public OpaqueDataHelper(String smName, boolean hasHistBufferConfigParam,
                            boolean hasKeywordIndicesParam,
                            boolean hasTimeStampParam) {
        mSmName = smName;
        mSmInfo = query(smName);
        mHasHistBufferConfigParam = hasHistBufferConfigParam;
        if (null != mSmInfo && mSmInfo.version == 0x0300) {
            mHasConfidenceLevelsParam = true;
        } else {
            mHasConfidenceLevelsParam = false;
        }
        mHasKeywordIndicesParam = hasKeywordIndicesParam;
        mHasTimeStampParam = hasTimeStampParam;
    }

    private int getTotalBufferSize() {
        int totalSize = 0;
        if (mHasConfidenceLevelsParam) {
            totalSize += SIZE_OF_ST_PARAM_HEADER + SIZE_OF_ST_CONFIDENCE_LEVELS_INFO;
        }

        if (mHasHistBufferConfigParam) {
            totalSize += SIZE_OF_ST_PARAM_HEADER + SIZE_OF_ST_HIST_BUFFER_INFO;
        }

        if (mHasKeywordIndicesParam) {
            totalSize += SIZE_OF_ST_PARAM_HEADER + SIZE_OF_ST_KEYWORD_INDICES_INFO;
        }

        if (mHasTimeStampParam) {
            totalSize += SIZE_OF_ST_PARAM_HEADER + SIZE_OF_TIME_STAMP_INFO;
        }

        return totalSize;
    }

    public byte[] fillOpaqueDataByteBuffer(int histDuration, int preRollDuration) {
        int OPAQUE_SIZE = 12;
        int MINOR_VERSION = 2;
        byte[] opaqueByteArray = new byte[OPAQUE_SIZE];

        // fill version
        opaqueByteArray[3] = (byte) (MINOR_VERSION >> 24 & 0xff);
        opaqueByteArray[2] = (byte) (MINOR_VERSION >> 16 & 0xff);
        opaqueByteArray[1] = (byte) (MINOR_VERSION >> 8 & 0xff);
        opaqueByteArray[0] = (byte) (MINOR_VERSION & 0xff);

        // fill histDuration
        opaqueByteArray[7] = (byte) (histDuration >> 24 & 0xff);
        opaqueByteArray[6] = (byte) (histDuration >> 16 & 0xff);
        opaqueByteArray[5] = (byte) (histDuration >> 8 & 0xff);
        opaqueByteArray[4] = (byte) (histDuration & 0xff);

        // fill pre-roll duration
        opaqueByteArray[11] = (byte) (preRollDuration >> 24 & 0xff);
        opaqueByteArray[10] = (byte) (preRollDuration >> 16 & 0xff);
        opaqueByteArray[9] = (byte) (preRollDuration >> 8 & 0xff);
        opaqueByteArray[8] = (byte) (preRollDuration & 0xff);

        return opaqueByteArray;
    }

    public byte[] fillOpaqueDataByteBuffer(int gmmKeyphraseLevel, int gmmUserLevel,
                                           int cnnKeyphraseLevel, int vopUserLevel,
                                           int histBufferDuration, int preRollDuration,
                                           int indicesStart, int indicesEnd) {
        byte[] buffer;
        int startPos = 0;
        int size = getTotalBufferSize();
        if (size > 0) {
            buffer = new byte[size];
            if (mHasConfidenceLevelsParam) {
                fillConfidenceLevelParam(buffer, 0, gmmKeyphraseLevel,
                        gmmUserLevel, cnnKeyphraseLevel, vopUserLevel);
                startPos += SIZE_OF_ST_PARAM_HEADER + SIZE_OF_ST_CONFIDENCE_LEVELS_INFO;
            }

            if (mHasHistBufferConfigParam) {
                fillHistBufferConfigParam(buffer, startPos, histBufferDuration, preRollDuration);
                startPos += SIZE_OF_ST_PARAM_HEADER + SIZE_OF_ST_HIST_BUFFER_INFO;
            }

            if (mHasKeywordIndicesParam) {
                fillKeywordIndicesParam(buffer, startPos, indicesStart, indicesEnd);
                startPos += SIZE_OF_ST_PARAM_HEADER + SIZE_OF_ST_KEYWORD_INDICES_INFO;
            }

            if (mHasTimeStampParam) {
                fillTimeStampParam(buffer, startPos);
            }

            return buffer;
        }
        return null;
    }

    private void fillKeywordIndicesParam(byte[] buffer, int startPos,
                                         int indicesStart, int indicesEnd) {
        // fill the key id
        int keyId = ST_Param_Key.ST_PARAM_KEY_KEYWORD_INDICES.ordinal();
        int startIndex = startPos;
        fillInt(buffer, startIndex, keyId);

        // fill the payload size
        int payloadSize = SIZE_OF_ST_KEYWORD_INDICES_INFO;
        startIndex = startPos + 4;
        fillInt(buffer, startIndex, payloadSize);

        // fill version
        int version = 1;
        startIndex = startPos + 8;
        fillInt(buffer, startIndex, version);

        // fill indices start index
        startIndex = startPos + 12;
        fillInt(buffer, startIndex, indicesStart);

        // fill indices end index
        startIndex = startPos + 16;
        fillInt(buffer, startIndex, indicesEnd);
    }

    private void fillTimeStampParam(byte[] buffer, int startPos) {
        // fill the key id
        int keyId = ST_Param_Key.ST_PARAM_KEY_TIMESTAMP.ordinal();
        int startIndex = startPos;
        fillInt(buffer, startIndex, keyId);

        // fill the payload size
        int payloadSize = SIZE_OF_TIME_STAMP_INFO;
        startIndex = startPos + 4;
        fillInt(buffer, startIndex, payloadSize);

        long timeStamp = System.currentTimeMillis();
        startIndex = startPos + 8;
        fillLong(buffer, startIndex, timeStamp);
    }

    private void fillHistBufferConfigParam(byte[] buffer, int startPos,
                                           int histBufferDuration, int preRollDuration) {
        // fill the key id
        int keyId = ST_Param_Key.ST_PARAM_KEY_HISTORY_BUFFER_CONFIG.ordinal();
        int startIndex = startPos;
        fillInt(buffer, startIndex, keyId);

        // fill the payload size
        int payloadSize = SIZE_OF_ST_HIST_BUFFER_INFO;
        startIndex = startPos + 4;
        fillInt(buffer, startIndex, payloadSize);

        // fill the version
        int histVersion = 2;
        startIndex = startPos + 8;
        fillInt(buffer, startIndex, histVersion);

        // fill the hist buffer duration
        startIndex = startPos + 12;
        fillInt(buffer, startIndex, histBufferDuration);

        // fill the pre-roll duration
        startIndex = startPos + 16;
        fillInt(buffer, startIndex, preRollDuration);
    }

    private void fillConfidenceLevelParam(byte[] buffer, int startPos,
                                          int gmmKeyphraseLevel, int gmmUserLevel,
                                          int cnnKeyphraseLevel, int vopUserLevel) {
        // fill the key id
        int keyId = ST_Param_Key.ST_PARAM_KEY_CONFIDENCE_LEVELS.ordinal();
        int startIndex = startPos;
        fillInt(buffer, startIndex, keyId);

        // fill the payload size
        int payloadSize = SIZE_OF_ST_CONFIDENCE_LEVELS_INFO;
        startIndex = startPos + 4;
        fillInt(buffer, startIndex, payloadSize);

        // fill the confidence level version
        startIndex = startPos + 8;
        fillInt(buffer, startIndex, CONF_LEVELS_INTF_VERSION_0002);

        // fill the sound model count
        int modelCount = mNumOfModels;
        startIndex = startPos + 12;
        fillInt(buffer, startIndex, modelCount);

        // match the userId with keyphraseRecognitionExtra in priority
        IExtendedSmModel extendedSmModel = Global.getInstance().getExtendedSmMgr()
                .getSoundModel(mSmName);
        SoundTrigger.KeyphraseRecognitionExtra[] extraArray
                = extendedSmModel.getKeyphraseRecognitionExtra();
        boolean bMatchWithExtra = false;
        if (null != extraArray && extraArray.length > 0) {
            bMatchWithExtra = true;
        }
        LogUtils.d(TAG, "fillConfidenceLevelParam: bMatchWithExtra = "
                + bMatchWithExtra);

        // fill sound model GMM confidence level
        if (mHasGMM) {
            int gmmSmId = ST_SM_ID_SVA_GMM;
            startIndex = startPos + 16;
            fillInt(buffer, startIndex, gmmSmId);

            // fill the keyphrase count
            int keyphraseCount = null != mSmInfo ? mSmInfo.keywordInfo.length : 0;
            if (bMatchWithExtra) {
                keyphraseCount = extraArray.length;
            }
            LogUtils.d(TAG, "fillConfidenceLevelParam: GMM keyphraseCount = "
                    + keyphraseCount);

            startIndex = startPos + 20;
            fillInt(buffer, startIndex, keyphraseCount);

            startIndex = startPos + 24;
            for (int ii = 0; ii < keyphraseCount; ii++) {
                int userCount = null != mSmInfo && null != mSmInfo.keywordInfo[ii]
                        ? mSmInfo.keywordInfo[ii].activeUsers.length : 0;
                LogUtils.d(TAG, "fillConfidenceLevelParam: GMM query userCount = "
                        + userCount);
                if (bMatchWithExtra) {
                    userCount = extraArray[ii].confidenceLevels.length;
                }
                LogUtils.d(TAG, "fillConfidenceLevelParam: GMM userCount = " + userCount);

                // fill the ii keyphrase level
                int pos = startIndex + ii * (4 + ST_MAX_CONFIDENCE_USERS * SIZE_OF_ST_USER_LEVEL);
                //buffer[pos] = (byte) (gmmKeyphraseLevel & 0xff);
                fillInt(buffer, pos, gmmKeyphraseLevel);

                // fill the user count
                pos = pos + 4;
                fillInt(buffer, pos, userCount);

                // fill the ii user level
                int jj = 0;
                if (bMatchWithExtra) {
                    for (SoundTrigger.ConfidenceLevel level : extraArray[ii].confidenceLevels) {
                        pos = pos + 4 + jj * SIZE_OF_ST_USER_LEVEL;

                        // fill user id
                        fillInt(buffer, pos, level.userId);

                        // fill user level
                        pos = pos + 4;
                        fillInt(buffer, pos, gmmUserLevel);
                        jj++;
                    }
                } else {
                    for (jj = 0; jj < userCount; jj++) {
                        pos = pos + 4 + jj * SIZE_OF_ST_USER_LEVEL;

                        // fill user id
                        fillInt(buffer, pos, jj);

                        // fill user level
                        pos = pos + 4;
                        fillInt(buffer, pos, gmmUserLevel);
                    }
                }
            }
        }

        if (mHasCNN) {
            int smId = ST_SM_ID_SVA_CNN;
            int newStartPos;
            if (mHasGMM) {
                newStartPos = startPos + SIZE_OF_ST_SOUND_MODEL_CONF_LEVELS;
            } else {
                newStartPos = startPos;
            }

            // fill model id
            startIndex = newStartPos + 16;
            fillInt(buffer, startIndex, smId);

            // fill the keyphrase count
            int cnnKeyphraseCount = null != mSmInfo ? mSmInfo.keywordInfo.length : 0;
            startIndex = newStartPos + 20;
            fillInt(buffer, startIndex, cnnKeyphraseCount);

            startIndex = newStartPos + 24;
            for (int iii = 0; iii < cnnKeyphraseCount; iii++) {
                int userCount = mSmInfo.keywordInfo[iii].activeUsers.length;

                // fill the ii keyphrase level
                int pos = startIndex + iii * (4 + ST_MAX_CONFIDENCE_USERS*SIZE_OF_ST_USER_LEVEL);
                //buffer[pos] = (byte) (cnnKeyphraseLevel & 0xff);
                fillInt(buffer, pos, cnnKeyphraseLevel);

                // fill the user count
                pos = pos + 4;
                fillInt(buffer, pos, userCount);

                // fill the iii user level
                for (int jjj = 0; jjj < userCount; jjj++) {
                    pos = pos + 4 + jjj * SIZE_OF_ST_USER_LEVEL;

                    // fill user id
                    fillInt(buffer, pos, jjj);

                    // fill user level
                    pos = pos + 4;
                    fillInt(buffer, pos, vopUserLevel);
                }
            }
        }

        if (mHasVOP) {
            int smId = ST_SM_ID_SVA_VOP;
            int newStartPos;
            if (mHasGMM) {
                if (mHasCNN) {
                    newStartPos = startPos + 2*SIZE_OF_ST_SOUND_MODEL_CONF_LEVELS;
                } else {
                    newStartPos = startPos + SIZE_OF_ST_SOUND_MODEL_CONF_LEVELS;
                }
            } else {
                if (mHasCNN) {
                    newStartPos = startPos + SIZE_OF_ST_SOUND_MODEL_CONF_LEVELS;
                } else {
                    newStartPos = startPos;
                }
            }

            // fill model id
            startIndex = newStartPos + 16;
            fillInt(buffer, startIndex, smId);

            // fill the keyphrase count
            int keyphraseCount = null != mSmInfo ? mSmInfo.keywordInfo.length : 0;
            startIndex = newStartPos + 20;
            fillInt(buffer, startIndex, keyphraseCount);

            startIndex = newStartPos + 24;
            for (int iii = 0; iii < keyphraseCount; iii++) {
                int userCount = mSmInfo.keywordInfo[iii].activeUsers.length;
                LogUtils.d(TAG, "fillConfidenceLevelParam: VOP query userCount = "
                        + userCount);
                if (bMatchWithExtra) {
                    userCount = extraArray[iii].confidenceLevels.length;
                }
                LogUtils.d(TAG, "fillConfidenceLevelParam: VOP userCount = " + userCount);

                // fill the iii keyphrase level
                int pos = startIndex + iii * (4 + ST_MAX_CONFIDENCE_USERS*SIZE_OF_ST_USER_LEVEL);
                //buffer[pos] = (byte) (cnnKeyphraseLevel & 0xff);
                fillInt(buffer, pos, cnnKeyphraseLevel);

                // fill the user count
                pos = pos + 4;
                fillInt(buffer, pos, userCount);

                // fill the iii user level
                for (int jjj = 0; jjj < userCount; jjj++) {
                    pos = pos + 4 + jjj * SIZE_OF_ST_USER_LEVEL;

                    // fill user id
                    fillInt(buffer, pos, jjj);

                    // fill user level
                    pos = pos + 4;
                    fillInt(buffer, pos, vopUserLevel);
                }
            }
        }
    }

    private void fillInt(byte[] buffer, final int startPos, int value) {
        int startIndex = startPos;
        buffer[startIndex] = (byte) (value & 0xff);
        buffer[++startIndex] = (byte) (value >> 8 & 0xff);
        buffer[++startIndex] = (byte) (value >> 16 & 0xff);
        buffer[++startIndex] = (byte) (value >> 24 & 0xff);
    }

    private void fillLong(byte[] buffer, final int startPos, long value) {
        int startIndex = startPos;
        buffer[startIndex] = (byte) (value & 0xff);
        buffer[++startIndex] = (byte) (value >> 8 & 0xff);
        buffer[++startIndex] = (byte) (value >> 16 & 0xff);
        buffer[++startIndex] = (byte) (value >> 24 & 0xff);
        buffer[++startIndex] = (byte) (value >> 32 & 0xff);
        buffer[++startIndex] = (byte) (value >> 40 & 0xff);
        buffer[++startIndex] = (byte) (value >> 48 & 0xff);
        buffer[++startIndex] = (byte) (value >> 56 & 0xff);
    }

    private ListenTypes.SVASoundModelInfo query(String smFullName) {
        LogUtils.d(TAG, "query: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.d(TAG, "query: invalid input param");
            return null;
        }

        String filePath = Global.PATH_ROOT + "/" + smFullName;
        if (FileUtils.isExist(filePath)) {
            try {
                ByteBuffer smBuffer;
                smBuffer = FileUtils.readFileToByteBuffer(filePath);
                return (ListenTypes.SVASoundModelInfo) ListenSoundModel.query(smBuffer);
            } catch (IOException e) {
                LogUtils.d(TAG, "query: file IO exception");
                e.printStackTrace();
                return null;
            }
        } else {
            LogUtils.d(TAG, "query: error file not exists");
            return null;
        }
    }

    enum ST_Param_Key {
        ST_PARAM_KEY_CONFIDENCE_LEVELS,
        ST_PARAM_KEY_HISTORY_BUFFER_CONFIG,
        ST_PARAM_KEY_KEYWORD_INDICES,
        ST_PARAM_KEY_TIMESTAMP,
    }
}

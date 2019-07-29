/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;

import android.content.Intent;
import android.content.SharedPreferences;

import com.qualcomm.qti.sva.utils.LogUtils;

import java.net.URISyntaxException;

final class SettingsDAO {
    private final static String TAG = SettingsDAO.class.getSimpleName();

    // setting item key definition
    private static final String KEY_GLOBAL_SM3_GMM_KEYPHRASE_CONFIDENCE_LEVEL
            = "global_sm3_gmm_keyphrase_confidence_level";
    private static final String KEY_GLOBAL_SM2_GMM_KEYPHRASE_CONFIDENCE_LEVEL
            = "global_sm2_gmm_keyphrase_confidence_level";
    private static final String KEY_GLOBAL_SM3_GMM_USER_CONFIDENCE_LEVEL
            = "global_sm3_gmm_user_confidence_level";
    private static final String KEY_GLOBAL_SM2_GMM_USER_CONFIDENCE_LEVEL
            = "global_sm2_gmm_user_confidence_level";
    private static final String KEY_GLOBAL_SM3_CNN_KEYPHRASE_CONFIDENCE_LEVEL
            = "global_sm3_cnn_keyphrase_confidence_level";
    private static final String KEY_GLOBAL_SM3_VOP_USER_CONFIDENCE_LEVEL
            = "global_sm3_vop_user_confidence_level";
    private static final String KEY_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL
            = "global_gmm_training_confidence_level";
    private static final String KEY_GMM_KEYPHRASE_CONFIDENCE_LEVEL
            = "gmm_keyphrase_confidence_level";
    private static final String KEY_GMM_USER_CONFIDENCE_LEVEL
            = "gmm_user_confidence_level";
    private static final String KEY_CNN_KEYPHRASE_CONFIDENCE_LEVEL
            = "cnn_keyphrase_confidence_level";
    private static final String KEY_VOP_USER_CONFIDENCE_LEVEL
            = "vop_user_confidence_level";
    private static final String KEY_GLOBAL_DETECTION_TONE_ENABLED
            = "global_detection_tone_enabled";
    private static final String KEY_GLOBAL_IS_DISPLAY_ADVANCED_DETAILS
            = "global_is_display_advanced_details";
    private static final String KEY_USER_VERIFICATION_ENABLED
            = "user_verification_enabled";
    private static final String KEY_VOICE_REQUEST_ENABLED
            = "voice_request_enabled";
    private static final String KEY_VOICE_REQUEST_LENGTH_IN_MILLISEC
            = "voice_request_len_in_millisec";
    private static final String KEY_OPAQUE_DATA_TRANSFER_ENABLED
            = "opaque_data_transfer_enabled";
    private static final String KEY_HIST_BUFFER_TIME_IN_MILLI_SEC
            = "hist_buffer_time_in_milli_sec";
    private static final String KEY_PRE_ROLL_DURATION_IN_MILLI_SEC
            = "pre_roll_duration_in_milli_sec";
    private static final String KEY_ACTION_NAME = "action_name";
    private static final String KEY_ACTION_INTENT = "action_intent";

    // setting item default value definition
    private static final int DEFAULT_GLOBAL_SM3_GMM_KEYPHRASE_CONFIDENCE_LEVEL = 40;
    private static final int DEFAULT_GLOBAL_SM3_GMM_USER_CONFIDENCE_LEVEL = 1;
    private static final int DEFAULT_GLOBAL_SM3_CNN_KEYPHRASE_CONFIDENCE_LEVEL = 40;
    private static final int DEFAULT_GLOBAL_SM3_VOP_USER_CONFIDENCE_LEVEL = 35;

    private static final int DEFAULT_GLOBAL_SM2_GMM_KEYPHRASE_CONFIDENCE_LEVEL = 69;
    private static final int DEFAULT_GLOBAL_SM2_GMM_USER_CONFIDENCE_LEVEL = 69;

    // default NiHaoXiaoLong model is difficult to be re-trained,
    // need set this value to lower value
    // so change default training confidence level to 50
    private static final int DEFAULT_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL = 50;
    private static final boolean DEFAULT_DETECTION_TONE_ENABLED = true;
    private static final boolean DEFAULT_IS_DISPLAY_ADVANCED_DETAILS = true;
    private static final boolean DEFAULT_USER_VERIFICATION_ENABLED = true;
    private static final boolean DEFAULT_VOICE_REQUEST_ENABLED = false;
    private static final int DEFAULT_VOICE_REQUEST_LENGTH = 3000;
    private static final boolean DEFAULT_OPAQUE_DATA_TRANSFER_ENABLED = false;
    private static final int DEFAULT_HIST_BUFFER_TIME = 2000;
    private static final int DEFAULT_PRE_ROLL_DURATION = 500;
    private static final String DEFAULT_ACTION_NAME = "None";
    private static final Intent DEFAULT_ACTION_INTENT = null;

    private static final int DEFAULT_GMM_KEYPHRASE_TYPE = 1;
    private static final int DEFAULT_GMM_USER_TYPE =2;
    private static final int DEFAULT_CNN_KEYPHRASE_TYPE = 3;
    private static final int DEFAULT_VOP_USER_TYPE = 4;


    private SettingsDAO() {
    }

    static int getGlobalSM3GMMKeyphraseConfidenceLevel(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalSM3GMMKeyphraseConfidenceLevel: invalid param");
            return DEFAULT_GLOBAL_SM3_GMM_KEYPHRASE_CONFIDENCE_LEVEL;
        }

        return sharedPrefs.getInt(KEY_GLOBAL_SM3_GMM_KEYPHRASE_CONFIDENCE_LEVEL,
                DEFAULT_GLOBAL_SM3_GMM_KEYPHRASE_CONFIDENCE_LEVEL);
    }

    static void setGlobalSM3GMMKeyphraseConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalGMMKeyphraseConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_GLOBAL_SM3_GMM_KEYPHRASE_CONFIDENCE_LEVEL, level).apply();
    }

    static int getGlobalSM3GMMUserConfidenceLevel(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalGMMUserConfidenceLevel: invalid param");
            return DEFAULT_GLOBAL_SM3_GMM_USER_CONFIDENCE_LEVEL;
        }

        return sharedPrefs.getInt(KEY_GLOBAL_SM3_GMM_USER_CONFIDENCE_LEVEL,
                DEFAULT_GLOBAL_SM3_GMM_USER_CONFIDENCE_LEVEL);
    }

    static void setGlobalSM3GMMUserConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalGMMUserConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_GLOBAL_SM3_GMM_USER_CONFIDENCE_LEVEL, level).apply();
    }

    static int getGlobalSM3CNNKeyphraseConfidenceLevel(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalCNNKeyphraseConfidenceLevel: invalid param");
            return DEFAULT_GLOBAL_SM3_CNN_KEYPHRASE_CONFIDENCE_LEVEL;
        }

        return sharedPrefs.getInt(KEY_GLOBAL_SM3_CNN_KEYPHRASE_CONFIDENCE_LEVEL,
                DEFAULT_GLOBAL_SM3_CNN_KEYPHRASE_CONFIDENCE_LEVEL);
    }

    static void setGlobalSM3CNNKeyphraseConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalCNNKeyphraseConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_GLOBAL_SM3_CNN_KEYPHRASE_CONFIDENCE_LEVEL, level).apply();
    }

    static int getGlobalSM3VOPUserConfidenceLevel(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalVOPUserConfidenceLevel: invalid param");
            return DEFAULT_GLOBAL_SM3_VOP_USER_CONFIDENCE_LEVEL;
        }

        return sharedPrefs.getInt(KEY_GLOBAL_SM3_VOP_USER_CONFIDENCE_LEVEL,
                DEFAULT_GLOBAL_SM3_VOP_USER_CONFIDENCE_LEVEL);
    }

    static void setGlobalSM3VOPUserConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalVOPUserConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_GLOBAL_SM3_VOP_USER_CONFIDENCE_LEVEL, level).apply();
    }

    static int getGlobalSM2GMMKeyphraseConfidenceLevel(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalSM3GMMKeyphraseConfidenceLevel: invalid param");
            return DEFAULT_GLOBAL_SM2_GMM_KEYPHRASE_CONFIDENCE_LEVEL;
        }

        return sharedPrefs.getInt(KEY_GLOBAL_SM2_GMM_KEYPHRASE_CONFIDENCE_LEVEL,
                DEFAULT_GLOBAL_SM2_GMM_KEYPHRASE_CONFIDENCE_LEVEL);
    }

    static void setGlobalSM2GMMKeyphraseConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalGMMKeyphraseConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_GLOBAL_SM2_GMM_KEYPHRASE_CONFIDENCE_LEVEL, level).apply();
    }

    static int getGlobalSM2GMMUserConfidenceLevel(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalGMMUserConfidenceLevel: invalid param");
            return DEFAULT_GLOBAL_SM2_GMM_USER_CONFIDENCE_LEVEL;
        }

        return sharedPrefs.getInt(KEY_GLOBAL_SM2_GMM_USER_CONFIDENCE_LEVEL,
                DEFAULT_GLOBAL_SM2_GMM_USER_CONFIDENCE_LEVEL);
    }

    static void setGlobalSM2GMMUserConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalGMMUserConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_GLOBAL_SM2_GMM_USER_CONFIDENCE_LEVEL, level).apply();
    }

    static int getGlobalGMMTrainingConfidenceLevel(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalGMMTrainingConfidenceLevel: invalid param");
            return DEFAULT_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL;
        }

        return sharedPrefs.getInt(KEY_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL,
                DEFAULT_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL);
    }

    static void setGlobalGMMTrainingConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalGMMTrainingConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL, level).apply();
    }

    static int getGMMKeyphraseConfidenceLevel(SharedPreferences sharedPrefs,
                                              SharedPreferences globalPrefs,
                                              ISmModel.MODEL_VERSION version) {
        if (null == sharedPrefs && null == globalPrefs) {
            LogUtils.d(TAG, "getGMMKeyphraseConfidenceLevel: invalid param");
            return getDefaultGlobalConfidenceLevel(DEFAULT_GMM_KEYPHRASE_TYPE, version);
        }

        return sharedPrefs.getInt(KEY_GMM_KEYPHRASE_CONFIDENCE_LEVEL,
                getGlobalConfidenceLevel(globalPrefs, DEFAULT_GMM_KEYPHRASE_TYPE, version));
    }

    static void setGMMKeyphraseConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGMMKeyphraseConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_GMM_KEYPHRASE_CONFIDENCE_LEVEL, level).apply();
    }

    static int getGMMUserConfidenceLevel(SharedPreferences sharedPrefs,
                                         SharedPreferences globalPrefs,
                                         ISmModel.MODEL_VERSION version) {
        if (null == sharedPrefs && null == globalPrefs) {
            LogUtils.d(TAG, "getGMMUserConfidenceLevel: invalid param");
            return getDefaultGlobalConfidenceLevel(DEFAULT_GMM_USER_TYPE, version);
        }

        return sharedPrefs.getInt(KEY_GMM_USER_CONFIDENCE_LEVEL,
                getGlobalConfidenceLevel(globalPrefs, DEFAULT_GMM_USER_TYPE, version));
    }

    static void setGMMUserConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGMMUserConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_GMM_USER_CONFIDENCE_LEVEL, level).apply();
    }

    static int getCNNKeyphraseConfidenceLevel(SharedPreferences sharedPrefs,
                                              SharedPreferences globalPrefs,
                                              ISmModel.MODEL_VERSION version) {
        if (null == sharedPrefs && null == globalPrefs) {
            LogUtils.d(TAG, "getCNNKeyphraseConfidenceLevel: invalid param");
            return getDefaultGlobalConfidenceLevel(DEFAULT_CNN_KEYPHRASE_TYPE, version);
        }

        return sharedPrefs.getInt(KEY_CNN_KEYPHRASE_CONFIDENCE_LEVEL,
                getGlobalConfidenceLevel(globalPrefs, DEFAULT_CNN_KEYPHRASE_TYPE, version));
    }

    static void setCNNKeyphraseConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setCNNKeyphraseConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_CNN_KEYPHRASE_CONFIDENCE_LEVEL, level).apply();
    }

    static int getVOPUserConfidenceLevel(SharedPreferences sharedPrefs,
                                         SharedPreferences globalPrefs,
                                         ISmModel.MODEL_VERSION version) {
        if (null == sharedPrefs && null == globalPrefs) {
            LogUtils.d(TAG, "getVOPUserConfidenceLevel: invalid param");
            return getDefaultGlobalConfidenceLevel(DEFAULT_VOP_USER_TYPE, version);
        }

        return sharedPrefs.getInt(KEY_VOP_USER_CONFIDENCE_LEVEL,
                getGlobalConfidenceLevel(globalPrefs, DEFAULT_VOP_USER_TYPE, version));
    }

    static void setVOPUserConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setVOPUserConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_VOP_USER_CONFIDENCE_LEVEL, level).apply();
    }

    static boolean getGlobalDetectionToneEnabled(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalDetectionToneEnabled: invalid param");
            return DEFAULT_DETECTION_TONE_ENABLED;
        }

        return sharedPrefs.getBoolean(KEY_GLOBAL_DETECTION_TONE_ENABLED,
                DEFAULT_DETECTION_TONE_ENABLED);
    }

    static void setGlobalDetectionToneEnabled(SharedPreferences sharedPrefs, boolean enabled) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalDetectionToneEnabled: invalid param");
            return;
        }

        sharedPrefs.edit().putBoolean(KEY_GLOBAL_DETECTION_TONE_ENABLED, enabled).apply();
    }

    static boolean getGlobalIsDisplayAdvancedDetails(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalIsDisplayAdvancedDetails: invalid param");
            return DEFAULT_IS_DISPLAY_ADVANCED_DETAILS;
        }

        return sharedPrefs.getBoolean(KEY_GLOBAL_IS_DISPLAY_ADVANCED_DETAILS,
                DEFAULT_IS_DISPLAY_ADVANCED_DETAILS);
    }

    static void setGlobalIsDisplayAdvancedDetails(SharedPreferences sharedPrefs,
                                                  boolean bDisplay) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalIsDisplayAdvancedDetails: invalid param");
            return;
        }

        sharedPrefs.edit().putBoolean(KEY_GLOBAL_IS_DISPLAY_ADVANCED_DETAILS, bDisplay).apply();
    }

    static boolean getUserVerificationEnabled(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getUserVerificationEnabled: invalid param");
            return DEFAULT_USER_VERIFICATION_ENABLED;
        }

        return sharedPrefs.getBoolean(KEY_USER_VERIFICATION_ENABLED,
                DEFAULT_USER_VERIFICATION_ENABLED);
    }

    static void setUserVerificationEnabled(SharedPreferences sharedPrefs, boolean enabled) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setUserVerificationEnabled: invalid param");
            return;
        }

        sharedPrefs.edit().putBoolean(KEY_USER_VERIFICATION_ENABLED, enabled).apply();
    }

    static boolean getVoiceRequestEnabled(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getVoiceRequestEnabled: invalid param");
            return DEFAULT_VOICE_REQUEST_ENABLED;
        }

        return sharedPrefs.getBoolean(KEY_VOICE_REQUEST_ENABLED,
                DEFAULT_VOICE_REQUEST_ENABLED);
    }

    static void setVoiceRequestEnabled(SharedPreferences sharedPrefs, boolean enabled) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setVoiceRequestEnabled: invalid param");
            return;
        }

        sharedPrefs.edit().putBoolean(KEY_VOICE_REQUEST_ENABLED, enabled).apply();
    }

    static int getVoiceRequestLength(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getVoiceRequestLength: invalid param");
            return DEFAULT_VOICE_REQUEST_LENGTH;
        }
        return sharedPrefs.getInt(KEY_VOICE_REQUEST_LENGTH_IN_MILLISEC,
                DEFAULT_VOICE_REQUEST_LENGTH);
    }

    static void setVoiceRequestLength(SharedPreferences sharedPrefs, int len) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setVoiceRequestLength: invalid param");
            return;
        }
        if (len > 0) {
            sharedPrefs.edit().putInt(KEY_VOICE_REQUEST_LENGTH_IN_MILLISEC, len).apply();
        }
    }

    static boolean getOpaqueDataTransferEnabled(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getOpaqueDataTransferEnabled: invalid param");
            return DEFAULT_OPAQUE_DATA_TRANSFER_ENABLED;
        }

        return sharedPrefs.getBoolean(KEY_OPAQUE_DATA_TRANSFER_ENABLED,
                DEFAULT_OPAQUE_DATA_TRANSFER_ENABLED);
    }

    static void setOpaqueDataTransferEnabled(SharedPreferences sharedPrefs, boolean enabled) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setOpaqueDataTransferEnabled: invalid param");
            return;
        }

        sharedPrefs.edit().putBoolean(KEY_OPAQUE_DATA_TRANSFER_ENABLED, enabled).apply();
    }

    static int getHistBufferTime(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getHistBufferTime: invalid param");
            return DEFAULT_HIST_BUFFER_TIME;
        }

        return sharedPrefs.getInt(KEY_HIST_BUFFER_TIME_IN_MILLI_SEC,
                DEFAULT_HIST_BUFFER_TIME);
    }

    static void setHistBufferTime(SharedPreferences sharedPrefs, int len) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setHistBufferTime: invalid param");
            return;
        }

        if (len >= 0) {
            sharedPrefs.edit().putInt(KEY_HIST_BUFFER_TIME_IN_MILLI_SEC, len).apply();
        }
    }

    static int getPreRollDuration(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getPreRollDuration: invalid param");
            return DEFAULT_PRE_ROLL_DURATION;
        }

        return sharedPrefs.getInt(KEY_PRE_ROLL_DURATION_IN_MILLI_SEC,
                DEFAULT_PRE_ROLL_DURATION);
    }

    static void setPreRollDuration(SharedPreferences sharedPrefs, int len) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setPreRollDuration: invalid param");
            return;
        }

        if (len >= 0) {
            sharedPrefs.edit().putInt(KEY_PRE_ROLL_DURATION_IN_MILLI_SEC, len).apply();
        }
    }

    public static void setActionName(SharedPreferences sharedPrefs, String actionName) {
        LogUtils.d(TAG, "setActionName: actionName = " + actionName);
        if (null == sharedPrefs || null == actionName) {
            LogUtils.d(TAG, "setActionName: invalid param");
            return;
        }

        sharedPrefs.edit().putString(KEY_ACTION_NAME, actionName).apply();
    }

    public static String getActionName(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getActionName: invalid param");
            return DEFAULT_ACTION_NAME;
        }

        return sharedPrefs.getString(KEY_ACTION_NAME, DEFAULT_ACTION_NAME);
    }

    public static Intent getActionIntent(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getActionIntent: invalid param");
            return DEFAULT_ACTION_INTENT;
        }

        Intent result;
        try {
            String action = sharedPrefs.getString(KEY_ACTION_INTENT, null);
            if (null != action) {
                result = Intent.parseUri(action, 0);
            } else {
                result = null;
            }
        } catch (URISyntaxException e) {
            e.printStackTrace();
            result = null;
        }

        LogUtils.d(TAG, "getActionIntent: result = " + result);
        return result;
    }

    public static void setActionIntent(SharedPreferences sharedPrefs, Intent actionIntent) {
        LogUtils.d(TAG, "setActionIntent: actionIntent = " + actionIntent);
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setActionIntent: invalid param");
            return;
        }

        if (null != actionIntent) {
            sharedPrefs.edit().putString(KEY_ACTION_INTENT, actionIntent.toUri(0)).apply();
        } else {
            sharedPrefs.edit().putString(KEY_ACTION_INTENT, null).apply();
        }
    }

    private static int getDefaultGlobalConfidenceLevel(int type,
                                                       ISmModel.MODEL_VERSION version) {
        int confidenceLevel = 0;
        switch(type) {
            case DEFAULT_GMM_KEYPHRASE_TYPE:
                if (version == ISmModel.MODEL_VERSION.VERSION_2_0) {
                    confidenceLevel = DEFAULT_GLOBAL_SM2_GMM_KEYPHRASE_CONFIDENCE_LEVEL;
                } else if(version == ISmModel.MODEL_VERSION.VERSION_3_0) {
                    confidenceLevel = DEFAULT_GLOBAL_SM3_GMM_KEYPHRASE_CONFIDENCE_LEVEL;
                }
                break;
            case DEFAULT_GMM_USER_TYPE:
                if (version == ISmModel.MODEL_VERSION.VERSION_2_0) {
                    confidenceLevel = DEFAULT_GLOBAL_SM2_GMM_USER_CONFIDENCE_LEVEL;
                } else if(version == ISmModel.MODEL_VERSION.VERSION_3_0) {
                    confidenceLevel = DEFAULT_GLOBAL_SM3_GMM_USER_CONFIDENCE_LEVEL;
                }
                break;
            case DEFAULT_CNN_KEYPHRASE_TYPE:
                confidenceLevel = DEFAULT_GLOBAL_SM3_CNN_KEYPHRASE_CONFIDENCE_LEVEL;
                break;
            case DEFAULT_VOP_USER_TYPE:
                confidenceLevel = DEFAULT_GLOBAL_SM3_VOP_USER_CONFIDENCE_LEVEL;
                break;
            default:
                break;
        }
        return confidenceLevel;
    }

    private static int getGlobalConfidenceLevel(SharedPreferences sharedPrefs, int type,
                                                ISmModel.MODEL_VERSION version) {
        int confidenceLevel = 0;
        switch(type) {
            case DEFAULT_GMM_KEYPHRASE_TYPE:
                if (version == ISmModel.MODEL_VERSION.VERSION_2_0) {
                    confidenceLevel = getGlobalSM2GMMKeyphraseConfidenceLevel(sharedPrefs);
                } else if(version == ISmModel.MODEL_VERSION.VERSION_3_0) {
                    confidenceLevel = getGlobalSM3GMMKeyphraseConfidenceLevel(sharedPrefs);
                }
                break;
            case DEFAULT_GMM_USER_TYPE:
                if (version == ISmModel.MODEL_VERSION.VERSION_2_0) {
                    confidenceLevel = getGlobalSM2GMMUserConfidenceLevel(sharedPrefs);
                } else if(version == ISmModel.MODEL_VERSION.VERSION_3_0) {
                    confidenceLevel = getGlobalSM3GMMUserConfidenceLevel(sharedPrefs);
                }
                break;
            case DEFAULT_CNN_KEYPHRASE_TYPE:
                confidenceLevel = getGlobalSM3CNNKeyphraseConfidenceLevel(sharedPrefs);
                break;
            case DEFAULT_VOP_USER_TYPE:
                confidenceLevel = getGlobalSM3VOPUserConfidenceLevel(sharedPrefs);
                break;
            default:
                break;
        }
        return confidenceLevel;
    }
}

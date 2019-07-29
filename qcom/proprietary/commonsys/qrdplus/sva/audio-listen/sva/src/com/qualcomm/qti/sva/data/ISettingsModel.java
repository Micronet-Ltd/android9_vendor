/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;

import android.content.Intent;

public interface ISettingsModel {
    String PREFIX_OF_SOUND_MODEL_PREFERENCES = "settings_";
    String NAME_OF_GLOBAL_PREFERENCES = "global_settings";

    int getGlobalSM3GMMKeyphraseConfidenceLevel();

    int getGlobalSM2GMMKeyphraseConfidenceLevel();

    void setGlobalSM3GMMKeyphraseConfidenceLevel(int level);

    void setGlobalSM2GMMKeyphraseConfidenceLevel(int level);

    int getGlobalSM3GMMUserConfidenceLevel();

    int getGlobalSM2GMMUserConfidenceLevel();

    void setGlobalSM3GMMUserConfidenceLevel(int level);

    void setGlobalSM2GMMUserConfidenceLevel(int level);

    int getGlobalSM3CNNKeyphraseConfidenceLevel();

    void setGlobalSM3CNNKeyphraseConfidenceLevel(int level);

    int getGlobalSM3VOPUserConfidenceLevel();

    void setGlobalSM3VOPUserConfidenceLevel(int level);

    int getGlobalGMMTrainingConfidenceLevel();

    void setGlobalGMMTrainingConfidenceLevel(int level);

    int getGMMKeyphraseConfidenceLevel();

    void setGMMKeyphraseConfidenceLevel(int level);

    int getGMMUserConfidenceLevel();

    void setGMMUserConfidenceLevel(int level);

    int getCNNKeyphraseConfidenceLevel();

    void setCNNKeyphraseConfidenceLevel(int level);

    int getVOPUserConfidenceLevel();

    void setVOPUserConfidenceLevel(int level);

    boolean getGlobalDetectionToneEnabled();

    void setGlobalDetectionToneEnabled(boolean enabled);

    boolean getGlobalIsDisplayAdvancedDetails();

    void setGlobalIsDisplayAdvancedDetails(boolean bDisplay);

    boolean getUserVerificationEnabled();

    void setUserVerificationEnabled(boolean enabled);

    boolean getVoiceRequestEnabled();

    void setVoiceRequestEnabled(boolean enabled);

    int getVoiceRequestLength();

    void setVoiceRequestLength(int len);

    boolean getOpaqueDataTransferEnabled();

    void setOpaqueDataTransferEnabled(boolean enabled);

    int getHistBufferTime();

    void setHistBufferTime(int len);

    int getPreRollDuration();

    void setPreRollDuration(int len);

    String getActionName();

    void setActionName(String actionName);

    Intent getActionIntent();

    void setActionIntent(Intent actionIntent);

}

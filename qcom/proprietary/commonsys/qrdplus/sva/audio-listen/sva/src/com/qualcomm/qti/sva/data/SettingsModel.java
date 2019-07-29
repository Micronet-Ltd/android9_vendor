/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;

import com.qualcomm.qti.sva.controller.ExtendedSmMgr;
import com.qualcomm.qti.sva.controller.Global;
import com.qualcomm.qti.sva.utils.LogUtils;
import com.qualcomm.qti.sva.utils.Utils;

public class SettingsModel implements ISettingsModel {
    private final String TAG = SettingsModel.class.getSimpleName();

    // variable define
    private Context mContext;
    private String mSmName;
    private SharedPreferences mSmPrefs;
    private SharedPreferences mGlobalPrefs;
    private ISmModel.MODEL_VERSION mSmVersion = ISmModel.MODEL_VERSION.VERSION_2_0;

    public SettingsModel(Context context, String smName) {
        mContext = context;
        mSmName = smName;

        if (null == smName) {
            mGlobalPrefs = getGlobalSharedPrefs(context);
        } else {
            mSmPrefs = getSmSharedPrefs(context, smName);
            mGlobalPrefs = getGlobalSharedPrefs(context);
        }
        ExtendedSmMgr smMgr = Global.getInstance().getExtendedSmMgr();
        IExtendedSmModel extendedSmModel = smMgr.getSoundModel(smName);
        if (extendedSmModel != null) {
            mSmVersion = extendedSmModel.getSoundModelVersion();
        }
    }

    private SharedPreferences getGlobalSharedPrefs(Context context) {
        String name = NAME_OF_GLOBAL_PREFERENCES;
        final Context storageContext;
        if (Utils.isAtLeastN()) {
            // All N devices have split storage areas. Migrate the existing preferences into
            // the new device encrypted storage area if that has not yet occurred.
            storageContext = context.createDeviceProtectedStorageContext();
            if (!storageContext.moveSharedPreferencesFrom(context, name)) {
                LogUtils.e(TAG, "Failed to migrate global shared preferences");
            }
        } else {
            storageContext = context;
        }

        return storageContext.getSharedPreferences(name, Context.MODE_PRIVATE);
    }

    private SharedPreferences getSmSharedPrefs(Context context, String smName) {
        String name = PREFIX_OF_SOUND_MODEL_PREFERENCES + smName;
        final Context storageContext;
        if (Utils.isAtLeastN()) {
            // All N devices have split storage areas. Migrate the existing preferences into
            // the new device encrypted storage area if that has not yet occurred.
            storageContext = context.createDeviceProtectedStorageContext();
            if (!storageContext.moveSharedPreferencesFrom(context, name)) {
                LogUtils.e(TAG, "Failed to migrate sound model shared preferences");
            }
        } else {
            storageContext = context;
        }

        return storageContext.getSharedPreferences(name, Context.MODE_PRIVATE);
    }

    @Override
    public int getGlobalSM3GMMKeyphraseConfidenceLevel() {
        return SettingsDAO.getGlobalSM3GMMKeyphraseConfidenceLevel(mGlobalPrefs);
    }

    @Override
    public int getGlobalSM2GMMKeyphraseConfidenceLevel() {
        return SettingsDAO.getGlobalSM2GMMKeyphraseConfidenceLevel(mGlobalPrefs);
    }

    @Override
    public void setGlobalSM3GMMKeyphraseConfidenceLevel(int level) {
        SettingsDAO.setGlobalSM3GMMKeyphraseConfidenceLevel(mGlobalPrefs, level);
    }

    @Override
    public void setGlobalSM2GMMKeyphraseConfidenceLevel(int level) {
        SettingsDAO.setGlobalSM2GMMKeyphraseConfidenceLevel(mGlobalPrefs, level);
    }

    @Override
    public int getGlobalSM3GMMUserConfidenceLevel() {
        return SettingsDAO.getGlobalSM3GMMUserConfidenceLevel(mGlobalPrefs);
    }

    @Override
    public int getGlobalSM2GMMUserConfidenceLevel() {
        return SettingsDAO.getGlobalSM2GMMUserConfidenceLevel(mGlobalPrefs);
    }

    @Override
    public void setGlobalSM3GMMUserConfidenceLevel(int level) {
        SettingsDAO.setGlobalSM3GMMUserConfidenceLevel(mGlobalPrefs, level);
    }

    @Override
    public void setGlobalSM2GMMUserConfidenceLevel(int level) {
        SettingsDAO.setGlobalSM2GMMUserConfidenceLevel(mGlobalPrefs, level);
    }

    @Override
    public int getGlobalSM3CNNKeyphraseConfidenceLevel() {
        return SettingsDAO.getGlobalSM3CNNKeyphraseConfidenceLevel(mGlobalPrefs);
    }

    @Override
    public void setGlobalSM3CNNKeyphraseConfidenceLevel(int level) {
        SettingsDAO.setGlobalSM3CNNKeyphraseConfidenceLevel(mGlobalPrefs, level);
    }

    @Override
    public int getGlobalSM3VOPUserConfidenceLevel() {
        return SettingsDAO.getGlobalSM3VOPUserConfidenceLevel(mGlobalPrefs);
    }

    @Override
    public void setGlobalSM3VOPUserConfidenceLevel(int level) {
        SettingsDAO.setGlobalSM3VOPUserConfidenceLevel(mGlobalPrefs, level);
    }

    @Override
    public int getGlobalGMMTrainingConfidenceLevel() {
        return SettingsDAO.getGlobalGMMTrainingConfidenceLevel(mGlobalPrefs);
    }

    @Override
    public void setGlobalGMMTrainingConfidenceLevel(int level) {
        SettingsDAO.setGlobalGMMTrainingConfidenceLevel(mGlobalPrefs, level);
    }

    @Override
    public int getGMMKeyphraseConfidenceLevel() {
        return SettingsDAO.getGMMKeyphraseConfidenceLevel(mSmPrefs, mGlobalPrefs,mSmVersion);
    }

    @Override
    public void setGMMKeyphraseConfidenceLevel(int level) {
        SettingsDAO.setGMMKeyphraseConfidenceLevel(mSmPrefs, level);
    }

    @Override
    public int getGMMUserConfidenceLevel() {
        return SettingsDAO.getGMMUserConfidenceLevel(mSmPrefs, mGlobalPrefs, mSmVersion);
    }

    @Override
    public void setGMMUserConfidenceLevel(int level) {
        SettingsDAO.setGMMUserConfidenceLevel(mSmPrefs, level);
    }

    @Override
    public int getCNNKeyphraseConfidenceLevel() {
        return SettingsDAO.getCNNKeyphraseConfidenceLevel(mSmPrefs, mGlobalPrefs, mSmVersion);
    }

    @Override
    public void setCNNKeyphraseConfidenceLevel(int level) {
        SettingsDAO.setCNNKeyphraseConfidenceLevel(mSmPrefs, level);
    }

    @Override
    public int getVOPUserConfidenceLevel() {
        return SettingsDAO.getVOPUserConfidenceLevel(mSmPrefs, mGlobalPrefs, mSmVersion);
    }

    @Override
    public void setVOPUserConfidenceLevel(int level) {
        SettingsDAO.setVOPUserConfidenceLevel(mSmPrefs, level);
    }

    @Override
    public boolean getGlobalDetectionToneEnabled() {
        return SettingsDAO.getGlobalDetectionToneEnabled(mGlobalPrefs);
    }

    @Override
    public void setGlobalDetectionToneEnabled(boolean enabled) {
        SettingsDAO.setGlobalDetectionToneEnabled(mGlobalPrefs, enabled);
    }

    @Override
    public boolean getGlobalIsDisplayAdvancedDetails() {
        return SettingsDAO.getGlobalIsDisplayAdvancedDetails(mGlobalPrefs);
    }

    @Override
    public void setGlobalIsDisplayAdvancedDetails(boolean bDisplay) {
        SettingsDAO.setGlobalIsDisplayAdvancedDetails(mGlobalPrefs, bDisplay);
    }

    @Override
    public boolean getUserVerificationEnabled() {
        return SettingsDAO.getUserVerificationEnabled(mSmPrefs);
    }

    @Override
    public void setUserVerificationEnabled(boolean enabled) {
        SettingsDAO.setUserVerificationEnabled(mSmPrefs, enabled);
    }

    @Override
    public boolean getVoiceRequestEnabled() {
        return SettingsDAO.getVoiceRequestEnabled(mSmPrefs);
    }

    @Override
    public void setVoiceRequestEnabled(boolean enabled) {
        SettingsDAO.setVoiceRequestEnabled(mSmPrefs, enabled);
    }

    @Override
    public int getVoiceRequestLength() {
        return SettingsDAO.getVoiceRequestLength(mSmPrefs);
    }

    @Override
    public void setVoiceRequestLength(int len) {
        SettingsDAO.setVoiceRequestLength(mSmPrefs, len);
    }

    @Override
    public boolean getOpaqueDataTransferEnabled() {
        return SettingsDAO.getOpaqueDataTransferEnabled(mSmPrefs);
    }

    @Override
    public void setOpaqueDataTransferEnabled(boolean enabled) {
        SettingsDAO.setOpaqueDataTransferEnabled(mSmPrefs, enabled);
    }

    @Override
    public int getHistBufferTime() {
        return SettingsDAO.getHistBufferTime(mSmPrefs);
    }

    @Override
    public void setHistBufferTime(int len) {
        SettingsDAO.setHistBufferTime(mSmPrefs, len);
    }

    @Override
    public int getPreRollDuration() {
        return SettingsDAO.getPreRollDuration(mSmPrefs);
    }

    @Override
    public void setPreRollDuration(int len) {
        SettingsDAO.setPreRollDuration(mSmPrefs, len);
    }

    @Override
    public String getActionName() {
        return SettingsDAO.getActionName(mSmPrefs);
    }

    @Override
    public void setActionName(String actionName) {
        SettingsDAO.setActionName(mSmPrefs, actionName);
    }

    @Override
    public Intent getActionIntent() {
        return SettingsDAO.getActionIntent(mSmPrefs);
    }

    @Override
    public void setActionIntent(Intent actionIntent) {
        SettingsDAO.setActionIntent(mSmPrefs, actionIntent);
    }
}

/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.smartassistant.sva.controller;

import android.content.Context;
import android.content.Intent;

import com.qualcomm.listen.ListenSoundModel;
import com.qualcomm.listen.ListenTypes;
import com.qualcomm.qti.smartassistant.sva.data.ExtendedSmModel;
import com.qualcomm.qti.smartassistant.sva.data.IExtendedSmModel;
import com.qualcomm.qti.smartassistant.sva.data.IKeyphraseModel;
import com.qualcomm.qti.smartassistant.sva.data.ISmModel;
import com.qualcomm.qti.smartassistant.utils.FileUtils;
import com.qualcomm.qti.smartassistant.utils.LogUtils;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;

public class ExtendedSmMgr {
    private static final int SVA_APP_KEYPHRASE_ID_BASE = 400;
    private final String TAG = ExtendedSmMgr.class.getSimpleName();
    private ArrayList<IExtendedSmModel> mSoundModelList = new ArrayList<>();
    private ArrayList<String> mUIMKeyphraseList = new ArrayList<>();

    // adapt the STManager API limitation
    // different SM 's keyphrase, need assign different keyphrase ID
    // different client, need assign different keyphrase ID
    private int mKeyphraseIdCounter = 0;

    public ExtendedSmMgr() {

    }

    public void initSoundModels(Context context) {
        mSoundModelList.clear();
        mUIMKeyphraseList.clear();

        File smDir = new File(FileUtils.APP_PATH);
        if (null != smDir && smDir.exists()) {
            File[] files = smDir.listFiles(new FilenameFilter() {
                @Override
                public boolean accept(File file, String fileName) {
                    if (fileName.endsWith(ISmModel.SUFFIX_DEFAULT_SOUND_MODEL)
                            || fileName.endsWith(ISmModel.SUFFIX_TRAINED_SOUND_MODEL)) {
                        return true;
                    }
                    return false;
                }
            });

            if (null == files || 0 == files.length) {
                LogUtils.e(TAG, "initSoundModels: no sound model file found");
                return;
            }

            for (File item : files) {
                String smFullFileName = item.getName();
                addSoundModel(context, smFullFileName);
            }
        }

        // set mUIMKeyphraseList to every IExtendedSmModel, used to differentiate UDK from PDK UV
        for (IExtendedSmModel item : mSoundModelList) {
            item.setUIMKeyphraseList(mUIMKeyphraseList);
        }
    }

    public ArrayList<IExtendedSmModel> getAllSoundModelList() {
        return mSoundModelList;
    }

    public ArrayList<IExtendedSmModel> getSingleKeyphraseSoundModelList() {
        ArrayList<IExtendedSmModel> singleKeyphraseModelList = new ArrayList<>();
        for (IExtendedSmModel item : mSoundModelList) {
            if (item.getSoundModelKeyphraseList().size() == 1) {
                singleKeyphraseModelList.add(item);
            }
        }
        return singleKeyphraseModelList;
    }

    public ArrayList<IExtendedSmModel> getTrainableSoundModelList() {
        ArrayList<IExtendedSmModel> trainableSmList = new ArrayList<>();
        for (IExtendedSmModel item : mSoundModelList) {
            if (item.getSoundModelKeyphraseList().size() == 1
                    && ISmModel.SUFFIX_DEFAULT_SOUND_MODEL.equalsIgnoreCase(
                    item.getSoundModelSuffix())) {
                trainableSmList.add(item);
            }
        }
        return trainableSmList;
    }

    public ArrayList<String> getUIMKeyphraseList() {
        return mUIMKeyphraseList;
    }

    public IExtendedSmModel getSoundModel(String smFullName) {
        LogUtils.d(TAG, "getSoundModel: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.d(TAG, "getSoundModel: invalid input param");
            return null;
        }

        IExtendedSmModel extendedSmModel = null;
        for (IExtendedSmModel item : mSoundModelList) {
            if (item.getSoundModelFullFileName().equalsIgnoreCase(smFullName)) {
                extendedSmModel = item;
                break;
            }
        }
        return extendedSmModel;
    }

    public void deleteSoundModel(String smFullName) {
        LogUtils.d(TAG, "deleteSoundModel: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.d(TAG, "deleteSoundModel: invalid input param");
            return;
        }
        IExtendedSmModel soundModel = getSoundModel(smFullName);
        if (null != soundModel) {
            mSoundModelList.remove(soundModel);
        }
    }

    public void addSoundModel(Context context, String smFullName) {
        LogUtils.d(TAG, "addSoundModel: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.d(TAG, "addSoundModel: invalid input param");
            return;
        }

        IExtendedSmModel soundModel = getSoundModel(smFullName);
        if (null == soundModel) {
            LogUtils.d(TAG, "addSoundModel: new one sound model");
            soundModel = new ExtendedSmModel(smFullName);
            mSoundModelList.add(soundModel);
        } else {
            LogUtils.d(TAG, "addSoundModel: sound model already exists");
        }

        updateSoundModel(context, soundModel);
    }

    private void updateSoundModel(Context context, IExtendedSmModel soundModel) {
        LogUtils.d(TAG, "updateSoundModel: soundModel = " + soundModel);
        if (null == soundModel) {
            LogUtils.e(TAG, "updateSoundModel: invalid input param");
            return;
        }

        String smFullName = soundModel.getSoundModelFullFileName();
        ListenTypes.SVASoundModelInfo modelInfo = query(smFullName);
        if (null != modelInfo) {
            soundModel.setSoundModelType(modelInfo.type);
            soundModel.setSoundModelVersion(convertVersion(modelInfo.version));
            if (null != modelInfo.keywordInfo && modelInfo.keywordInfo.length > 0) {
                soundModel.setSoundModelPrettyKeyphrase(modelInfo.keywordInfo[0].keywordPhrase);
                addKeyphraseAndUsers(context, modelInfo, soundModel);
            }
        }
    }

    private void addKeyphraseAndUsers(Context context, ListenTypes.SVASoundModelInfo modelInfo,
                                      IExtendedSmModel soundModel) {
        if (null == modelInfo || null == soundModel) {
            LogUtils.e(TAG, "addKeyphraseAndUsers: invalid input param");
            return;
        }

        ArrayList<String> indicesArr = generateKeyphraseUserIndicesArray(modelInfo);
        if (!indicesArr.isEmpty()) {
            for (ListenTypes.KeywordInfo keywordItem : modelInfo.keywordInfo) {
                int keyphraseId = generateKeyphraseId(context, indicesArr,
                        keywordItem.keywordPhrase);
                soundModel.addKeyphrase(keywordItem.keywordPhrase, keyphraseId);

                LogUtils.e(TAG, "addKeyphraseAndUsers: keyphrase " + keywordItem
                        + " 's keyphraseId = " + keyphraseId);

                // add the UIM model keyphrase to list
                if (!soundModel.isUserTrainedSoundModel()) {
                    if (!mUIMKeyphraseList.contains(keywordItem.keywordPhrase)) {
                        LogUtils.e(TAG, "addKeyphraseAndUsers: add "
                                + keywordItem.keywordPhrase + " to mUIMKeyphraseList");
                        mUIMKeyphraseList.add(keywordItem.keywordPhrase);
                    }
                }

                int userId = 0;
                for (String userItem : keywordItem.activeUsers) {
                    // adapt the STManager API limitation
                    userId++;
                    soundModel.addKeyphraseAndUser(keywordItem.keywordPhrase, keyphraseId,
                            userItem, userId);

                    LogUtils.e(TAG, "addKeyphraseAndUsers: user " + userItem
                            + " 's userId = " + userId);
                }
            }
        }
    }

    private int generateUserId(ArrayList<String> indicesArray, String keyphrase, String userName) {
        if (null == indicesArray || indicesArray.isEmpty()
                || null == keyphrase || null == userName) {
            LogUtils.e(TAG, "generateUserId: invalid input param");
            return IKeyphraseModel.INVALID_ID;
        }

        return indicesArray.indexOf(userName + "|" + keyphrase);
    }

    private int generateKeyphraseId(Context context, ArrayList<String> indicesArray,
                                    String keyphrase) {
        if (null == indicesArray || indicesArray.isEmpty() || null == keyphrase) {
            LogUtils.e(TAG, "generateKeyphraseId: invalid input param");
            return IKeyphraseModel.INVALID_ID;
        }

        // adapt the STManager API limitation
        int keyphraseId = SVA_APP_KEYPHRASE_ID_BASE;
        keyphraseId = keyphraseId + mKeyphraseIdCounter;
        mKeyphraseIdCounter++;
        LogUtils.e(TAG, "generateKeyphraseId: STManager API, keyphrase = " + keyphrase
                + " keyphraseId = " + keyphraseId);
        return keyphraseId;
    }

    private ArrayList<String> generateKeyphraseUserIndicesArray(
            ListenTypes.SVASoundModelInfo modelInfo) {
        ArrayList<String> indicesArray = new ArrayList<>();
        if (null == modelInfo) {
            LogUtils.e(TAG, "generateKeyphraseUserIndicesArray: invalid input param");
            return indicesArray;
        }

        for (ListenTypes.KeywordInfo keywordItem : modelInfo.keywordInfo) {
            indicesArray.add(keywordItem.keywordPhrase);
            LogUtils.d(TAG, "generateKeyphraseUserIndicesArray: "
                    + keywordItem.keywordPhrase + " index = " + (indicesArray.size() - 1));
        }

        for (String userNameItem : modelInfo.userNames) {
            for (ListenTypes.KeywordInfo keywordItem : modelInfo.keywordInfo) {
                for (String userItem : keywordItem.activeUsers) {
                    if (userNameItem.equals(userItem)) {
                        indicesArray.add(userNameItem + "|" + keywordItem.keywordPhrase);
                        LogUtils.d(TAG, "generateKeyphraseUserIndicesArray: "
                                + keywordItem.keywordPhrase + " & " + userNameItem
                                + " index = " + (indicesArray.size() - 1));
                    }
                }
            }
        }

        return indicesArray;
    }

    // this is only available for STManager API
    public String getSmNameByKeyphraseId(int keyphraseId) {
        String smName = null;
        if (keyphraseId < 0) {
            LogUtils.e(TAG, "getSmNameByKeyphraseId: invalid input param");
            return null;
        }

        for (IExtendedSmModel item : mSoundModelList) {
            for (IKeyphraseModel keyphraseItem : item.getSoundModelKeyphraseList()) {
                if (keyphraseId == keyphraseItem.getKeyphraseId()) {
                    smName = item.getSoundModelFullFileName();
                    break;
                }
            }
        }

        LogUtils.e(TAG, "getSmNameByKeyphraseId: keyphraseId = " + keyphraseId
                + " smName = " + smName);

        return smName;
    }

    private IExtendedSmModel.MODEL_VERSION convertVersion(int version) {
        IExtendedSmModel.MODEL_VERSION convertedVersion;
        switch (version) {
            case IExtendedSmModel.VERSION_0100:
                convertedVersion = IExtendedSmModel.MODEL_VERSION.VERSION_1_0;
                break;
            case IExtendedSmModel.VERSION_0200:
                convertedVersion = IExtendedSmModel.MODEL_VERSION.VERSION_2_0;
                break;
            case IExtendedSmModel.VERSION_0300:
                convertedVersion = IExtendedSmModel.MODEL_VERSION.VERSION_3_0;
                break;
            default:
                convertedVersion = IExtendedSmModel.MODEL_VERSION.VERSION_UNKNOWN;
                break;
        }

        LogUtils.d(TAG, "convertVersion: version = " + version
                + " convertedVersion = " + convertedVersion);

        return convertedVersion;
    }

    private ListenTypes.SVASoundModelInfo query(String smFullName) {
        LogUtils.d(TAG, "query: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.e(TAG, "query: invalid input param");
            return null;
        }

        String filePath = getSoundModelFullPath(smFullName);
        if (FileUtils.isFileExist(filePath)) {
            try {
                ByteBuffer smBuffer;
                smBuffer = FileUtils.readFileToByteBuffer(filePath);
                return (ListenTypes.SVASoundModelInfo) ListenSoundModel.query(smBuffer);
            } catch (IOException e) {
                LogUtils.e(TAG, "query: file IO exception");
                e.printStackTrace();
                return null;
            }
        } else {
            LogUtils.e(TAG, "query: error file not exists");
            return null;
        }
    }

    public String getSoundModelFullPath(String smFullName) {
        LogUtils.d(TAG, "getSoundModelFullPath: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.e(TAG, "getSoundModelFullPath: invalid input param");
            return null;
        }
        return FileUtils.APP_PATH + "/" + smFullName;
    }
}

/*
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.smartassistant.training;

import android.util.Log;

import com.qualcomm.listen.ListenTypes;
import com.qualcomm.qti.smartassistant.sva.SoundModelHelper;
import com.qualcomm.qti.smartassistant.utils.FileUtils;
import com.qualcomm.qti.smartassistant.sva.data.ISmModel;
import com.qualcomm.qti.smartassistant.utils.LogUtils;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;

public class SoundModelTrainer{
    private static final String TAG = "Training";

    private String mModelName;
    private SoundModelHelper mSoundModelHelper;
    private String mModelFileExtended = null;
    private String mModeFileToExtend = null;
    private ByteBuffer mExtendSoundModel = null;
    private ByteBuffer mSoundModelToModifyByteBuffer = null;
    private String mKeyphrase = null;
    private ListenTypes.ConfidenceData mConfidenceData = new ListenTypes.ConfidenceData();

    public SoundModelTrainer(String model) {
        mModelName = model;
        mSoundModelHelper = new SoundModelHelper();
        if (FileUtils.isFileExist(generateSoundModelFilePath(
                model + ISmModel.SUFFIX_TRAINED_SOUND_MODEL))) {
            mModeFileToExtend = model + ISmModel.SUFFIX_TRAINED_SOUND_MODEL;
        } else {
            mModeFileToExtend = model + ISmModel.SUFFIX_DEFAULT_SOUND_MODEL;
        }
        mModelFileExtended = model + ISmModel.SUFFIX_TRAINED_SOUND_MODEL;

        FileUtils.createDirIfNotExists(TrainingAssistant.PATH_TRAINING_RECORDINGS);
        ListenTypes.SVASoundModelInfo info = (ListenTypes.SVASoundModelInfo)
                (mSoundModelHelper.querySoundModelInfo(mModeFileToExtend));
        mKeyphrase = info.keywordInfo[0].keywordPhrase;
    }

    public String getExtendedSmName(){
        return mModelFileExtended;
    }

    public int verifyUserRecording() {
        int num = TrainingAssistant.getInstance().getNumUserRecordings();
        if (num < 1 || num > TrainingAssistant.NUM_TRAINING_RECORDINGS_REQUIRED) {
            return -1;
        }
        int confidenceLevel = mSoundModelHelper.verifyUserRecording(
                getSoundModelByteBufferFromName(mModeFileToExtend),
                mKeyphrase,
                TrainingAssistant.getInstance().getLastUserRecording());
        if (confidenceLevel <= 0) {
            Log.e(TAG, "verify recording failed with return status = " + confidenceLevel);
            return -1;
        }
        return confidenceLevel;
    }

    public boolean extendSoundModel(String username) {
        int size = mSoundModelHelper.getSizeofExtendedPdkModel(
                getSoundModelByteBufferFromName(mModeFileToExtend),
                mKeyphrase, username);
        if (size <= 0) {
            Log.e(TAG, "get size when extend, return error size = " + size);
            return false;
        }
        createExtendSoundModelByteBuffer(size);
        int returnStatus = mSoundModelHelper.createPdkUserModel(
                getSoundModelByteBufferFromName(mModeFileToExtend),
                mKeyphrase, username,
                TrainingAssistant.NUM_TRAINING_RECORDINGS_REQUIRED,
                TrainingAssistant.getInstance().getUserRecordings(),
                getExtendedSoundModel(),
                mConfidenceData
        );
        if (returnStatus == ListenTypes.STATUS_SUCCESS) {
            //TODO:
            saveExtendSoundModel();
            TrainingAssistant.getInstance().removeUserRecordings();
            return true;
        } else {

            return false;
        }
    }

    public boolean deleteUserFromSoundModel(String username) {
        File file = new File(generateSoundModelFilePath(mModelFileExtended));
        if (file.exists()){
            file.delete();
            LogUtils.d(TAG," deleteUserFromSoundModel delete udm file");
        }else {
            LogUtils.d(TAG," deleteUserFromSoundModel udm file not exist");
        }
        return true;
    }

    private void createExtendSoundModelByteBuffer(int size) {
        mExtendSoundModel = ByteBuffer.allocate(size);
    }

    private void createSoundModelToModifyByteBuffer(int size) {
        mSoundModelToModifyByteBuffer = ByteBuffer.allocate(size);
    }

    private ByteBuffer getExtendedSoundModel() {
        if (mExtendSoundModel == null) {
            Log.e(TAG, "must call createExtendSoundModelByteBuffer before get");
        }
        return mExtendSoundModel;
    }

    // returns null if no SM with specified name exists
    private ByteBuffer getSoundModelByteBufferFromName(String inSoundModelName) {
        if (soundModelFileExists(inSoundModelName)) {
            try {
                ByteBuffer soundModel = FileUtils.readFileToByteBuffer(
                        generateSoundModelFilePath(inSoundModelName));
                return soundModel;
            } catch (IOException e) {
                Log.e(TAG, "getKeywordOnlySoundModel: failed to read sound model: " + inSoundModelName);
                e.printStackTrace();
                return null;
            }
        } else {
            Log.v(TAG, "getSoundModelFromName: sound model with name: " + inSoundModelName +
                    " does not exist");
            return null;
        }
    }

    private boolean soundModelFileExists(String inSoundModelName) {
        File soundModelFile = new File(generateSoundModelFilePath(inSoundModelName));
        return soundModelFile.exists();
    }

    private String generateSoundModelFilePath(String inSoundModelName) {
        Log.v(TAG, "generateSoundModelFilePath: path= " + FileUtils.APP_PATH + "/" +
                inSoundModelName);
        return FileUtils.APP_PATH + "/" + inSoundModelName;
    }

    private void saveExtendSoundModel() {
        String filePath = generateSoundModelFilePath(mModelFileExtended);
        FileUtils.saveByteBufferToFile(this.mExtendSoundModel, filePath);
        mModeFileToExtend = mModelName + ISmModel.SUFFIX_TRAINED_SOUND_MODEL;
    }

    private void saveSoundModeToDeleteFrom() {
        String filePath = generateSoundModelFilePath(mModelFileExtended);
        FileUtils.saveByteBufferToFile(mSoundModelToModifyByteBuffer, filePath);
    }
}

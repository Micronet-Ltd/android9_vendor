/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.smartassistant.sva.session;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.hardware.soundtrigger.SoundTrigger;
import android.media.soundtrigger.SoundTriggerManager;

import com.qualcomm.qti.smartassistant.sva.controller.ExtendedSmMgr;
import com.qualcomm.qti.smartassistant.sva.data.IExtendedSmModel;
import com.qualcomm.qti.smartassistant.sva.data.ISmModel;
import com.qualcomm.qti.smartassistant.sva.data.IKeyphraseModel;
import com.qualcomm.qti.smartassistant.sva.data.IUserModel;
import com.qualcomm.qti.smartassistant.sva.opaque.OpaqueDataEncoder;
import com.qualcomm.qti.smartassistant.utils.LogUtils;
import com.qualcomm.qti.smartassistant.SmartAssistantApplication;
import com.qualcomm.qti.smartassistant.service.SmartAssistantService;
import com.qualcomm.qti.smartassistant.utils.SharedPreferenceUtils;
import com.qualcomm.qti.smartassistant.utils.FileUtils;
import com.qualcomm.qti.smartassistant.wrapper.AudioSystemWrapper;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.UUID;

public class WakeupSessionImpl implements IWakeupSession {
    private static final String TAG = WakeupSessionImpl.class.getSimpleName();
    private final SoundTriggerManager mSoundTriggerManager;
    private ExtendedSmMgr mExtendedSmMgr;

    private Context mAppContext;
    // QTI UUID
    private UUID mVendorUuid = UUID.fromString("68ab2d40-e860-11e3-95ef-0002a5d5c51b");
    private UUID mUuid = null;

    public WakeupSessionImpl(Context context) {
        mAppContext = context;
        mExtendedSmMgr = SmartAssistantApplication.getInstance().getExtendedSmMgr();
        mSoundTriggerManager = (SoundTriggerManager) context.getSystemService(
                Context.SOUND_TRIGGER_SERVICE);
    }

    @Override
    public int establishSvaSession(String smName) {
        int returnCode;
        LogUtils.d(TAG, "establishSvaSession: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "establishSvaSession: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        returnCode = loadSM(smName);
        if (RC_OK != returnCode) {
            LogUtils.d(TAG, "establishSvaSession: loadSM returnCode = " + returnCode);
            return returnCode;
        }

        returnCode = startRecognition(smName);
        if (RC_OK != returnCode) {
            LogUtils.d(TAG, "establishSvaSession: startRecognition returnCode = "
                    + returnCode);
            return returnCode;
        }

        return returnCode;
    }

    @Override
    public int terminateSvaSession(String smName) {
        int returnCode;
        LogUtils.d(TAG, "terminateSvaSession: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "terminateSvaSession: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        returnCode = stopRecognition(smName);
        if (RC_OK != returnCode) {
            LogUtils.d(TAG, "terminateSvaSession: stopRecognition returnCode = "
                    + returnCode);
            if (RC_WRONG_STATUS != returnCode) {
                return returnCode;
            }
        }

        returnCode = unloadSM(smName);
        if (RC_OK != returnCode) {
            LogUtils.d(TAG, "terminateSvaSession: unloadSM returnCode = "
                    + returnCode);
            return returnCode;
        }

        return returnCode;
    }

    @Override
    public void releaseAllSvaSessions() {
        ArrayList<IExtendedSmModel> allSoundModelListList
                = mExtendedSmMgr.getAllSoundModelList();
        int returnCode;
        for (IExtendedSmModel smModel : allSoundModelListList) {
            String smName = smModel.getSoundModelFullFileName();
            LogUtils.d(TAG, "releaseAllSvaSessions: smName = " + smName);
            switch (smModel.getSessionStatus()) {
                case UNLOADED:
                    LogUtils.d(TAG, "releaseAllSvaSessions: unloaded status, do nothing");
                    break;
                case STARTED:
                    returnCode = terminateSvaSession(smName);
                    LogUtils.d(TAG, "releaseAllSvaSessions: started status returnCode = "
                            + returnCode);
                    break;
                case LOADED:
                    returnCode = unloadSM(smName);
                    LogUtils.d(TAG, "releaseAllSvaSessions: loaded status returnCode = "
                            + returnCode);
                    break;
                case STOPPED:
                    returnCode = unloadSM(smName);
                    LogUtils.d(TAG, "releaseAllSvaSessions: stopped status returnCode = "
                            + returnCode);
                    break;
                default:
                    LogUtils.d(TAG, "releaseAllSvaSessions: invalid status value");
                    break;
            }

            // reset to initial status: UNLOADED
            smModel.setSessionStatus(IExtendedSmModel.SessionStatus.UNLOADED);
        }
    }

    @Override
    public int restartRecognition(String smName) {
        int returnCode;
        LogUtils.d(TAG, "restartRecognition: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "restartRecognition: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);

        if (IExtendedSmModel.SessionStatus.STARTED != extendedSmModel.getSessionStatus()) {
            LogUtils.d(TAG, "restartRecognition: failure for wrong status");
            returnCode = RC_FAILURE;
            return returnCode;
        }

        // call STM API
        UUID smUUID = extendedSmModel.getSoundModelUUID();
        returnCode = mSoundTriggerManager.startRecognition(smUUID, getPendingIntent(smName),
                extendedSmModel.getRecognitionConfig());
        if (RC_OK == returnCode) {
            LogUtils.d(TAG, "restartRecognition: STM restartRecognition success");
            extendedSmModel.setSessionStatus(IExtendedSmModel.SessionStatus.STARTED);
        }

        return returnCode;
    }

    private int loadSM(String smName) {
        int returnCode;
        LogUtils.d(TAG, "loadSM: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "loadSM: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        String filePath = mExtendedSmMgr.getSoundModelFullPath(smName);
        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);
        if (null == extendedSmModel || !FileUtils.isFileExist(filePath)) {
            LogUtils.d(TAG, "loadSM: sm file not exist");
            returnCode = RC_FILE_NOT_EXIST;
            return returnCode;
        }

        // if the sm have been loaded, return success directly
        if (IExtendedSmModel.SessionStatus.LOADED == extendedSmModel.getSessionStatus()
                || IExtendedSmModel.SessionStatus.STOPPED == extendedSmModel.getSessionStatus()) {
            LogUtils.d(TAG, "loadSM: already loaded, return success");
            returnCode = RC_OK;
            return returnCode;
        }

        // call STM API
        SoundTrigger.KeyphraseSoundModel keyphraseSoundModel = createSTKeyphraseSoundModel(smName);
        if (null == keyphraseSoundModel) {
            LogUtils.d(TAG, "loadSM: create KeyphraseSoundModel fail");
            returnCode = RC_FAILURE;
            return returnCode;
        }

        returnCode = mSoundTriggerManager.loadSoundModel(keyphraseSoundModel);
        if (RC_OK == returnCode) {
            LogUtils.d(TAG, "loadSM: STM loadSoundModel success");
            extendedSmModel.setSessionStatus(IExtendedSmModel.SessionStatus.LOADED);
        }

        return returnCode;
    }

    private SoundTrigger.KeyphraseSoundModel createSTKeyphraseSoundModel(String smName) {
        if (null == smName) {
            LogUtils.d(TAG, "createSTKeyphraseSoundModel: invalid input param");
            return null;
        }
        //determine recognition mode
        int recognitionMode = SoundTrigger.RECOGNITION_MODE_VOICE_TRIGGER;
        boolean isUserTrainedSM = mExtendedSmMgr.getSoundModel(smName).isUserTrainedSoundModel();
        if (isUserTrainedSM && SharedPreferenceUtils.getIsUserVerificationEnabled(mAppContext)) {
            recognitionMode += SoundTrigger.RECOGNITION_MODE_USER_IDENTIFICATION;
        }

        // inform HAL: the number of sessions that will be used.
        AudioSystemWrapper.setParameters("SVA_NUM_SESSIONS=1");

        ByteBuffer smBuffer = FileUtils.getSMData(smName);
        if (null == smBuffer) {
            LogUtils.d(TAG, "createSTKeyphraseSoundModel: read sm data fail");
            return null;
        }

        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);
        SoundTrigger.KeyphraseRecognitionExtra[] stKpRecognitionExtraArray;
        SoundTrigger.Keyphrase[] stKpArray;

        // resume the mVendorUuid to QTI UUID
        mVendorUuid = UUID.fromString("68ab2d40-e860-11e3-95ef-0002a5d5c51b");
        int keyphraseCount = extendedSmModel.getSoundModelKeyphraseList().size();
        // adapt the STManager API limitation
        // only support single keyphrase now
        if (keyphraseCount > 1) {
            LogUtils.d(TAG, "createSTKeyphraseSoundModel: this is MK Sm, "
                    + "only support single keyphrase now");
            keyphraseCount = 1;
        }

        stKpArray = new SoundTrigger.Keyphrase[keyphraseCount];
        stKpRecognitionExtraArray = new SoundTrigger.KeyphraseRecognitionExtra[keyphraseCount];
        for (int j = 0; j < keyphraseCount; j++) {
            IKeyphraseModel keyphraseModel
                    = extendedSmModel.getSoundModelKeyphraseList().get(j);

            // generate user id array
            int userCount = keyphraseModel.getUserList().size();
            int[] userIdList = new int[userCount];
            int index = 0;
            for (IUserModel item : keyphraseModel.getUserList()) {
                userIdList[index++] = item.getUserId();
            }

            // create SoundTrigger.Keyphrase object
            SoundTrigger.Keyphrase stKeyphrase
                    = new SoundTrigger.Keyphrase(keyphraseModel.getKeyphraseId(),
                    recognitionMode, "en_US", keyphraseModel.getKeyphraseFullName(),
                    userIdList);
            stKpArray[j] = stKeyphrase;

            // value user confidence levels
            ISmModel.MODEL_VERSION version = extendedSmModel.getSoundModelVersion();
            SoundTrigger.ConfidenceLevel[] stUserConfidenceLevelArray
                    = new SoundTrigger.ConfidenceLevel[userCount];
            for (int k = 0; k < userCount; k++) {
                int userConfidenceLevel;
                if (ISmModel.MODEL_VERSION.VERSION_3_0 == version) {
                    userConfidenceLevel = FileUtils.getVOPUserConfidenceLevelSetting();
                } else {
                    userConfidenceLevel = FileUtils.getUserConfidenceLevelSetting();
                }

                SoundTrigger.ConfidenceLevel stConfidenceLevel
                        = new SoundTrigger.ConfidenceLevel(stKeyphrase.users[k],
                        userConfidenceLevel);
                stUserConfidenceLevelArray[k] = stConfidenceLevel;
            }

            //value keyphrase confidence level
            int keyphraseConfidenceLevel;
            if (ISmModel.MODEL_VERSION.VERSION_3_0 == version) {
                keyphraseConfidenceLevel = FileUtils.getCNNKeyphraseConfidenceLevelSetting();
            } else {
                keyphraseConfidenceLevel = FileUtils.getKeyphraseConfidenceLevelSetting();
            }

            // create KeyphraseRecognitionExtra
            SoundTrigger.KeyphraseRecognitionExtra stKpRecognitionExtra
                    = new SoundTrigger.KeyphraseRecognitionExtra(
                    keyphraseModel.getKeyphraseId(), recognitionMode,
                    keyphraseConfidenceLevel, stUserConfidenceLevelArray);
            stKpRecognitionExtraArray[j] = stKpRecognitionExtra;
        }


        // save stKpRecognitionExtraArray which is used for startRecognition param
        extendedSmModel.setKeyphraseRecognitionExtra(stKpRecognitionExtraArray);

        smBuffer.rewind();
        byte[] smByteArray = new byte[smBuffer.remaining()];
        smBuffer.get(smByteArray);

        SoundTrigger.KeyphraseSoundModel stKpSm = new SoundTrigger.KeyphraseSoundModel(
                extendedSmModel.getSoundModelUUID(), mVendorUuid, smByteArray, stKpArray);

        return stKpSm;
    }

    private int startRecognition(String smName) {
        int returnCode;
        LogUtils.d(TAG, "startRecognition: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "startRecognition: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        // judge the sm whether loaded or not
        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);
        if (IExtendedSmModel.SessionStatus.STARTED == extendedSmModel.getSessionStatus()) {
            LogUtils.d(TAG, "startRecognition: already started, return success directly");
            returnCode = RC_OK;
            return returnCode;
        }

        boolean bCanTransit = isStateTransitional(extendedSmModel.getSessionStatus(),
                IExtendedSmModel.SessionStatus.STARTED);
        if (!bCanTransit) {
            LogUtils.d(TAG, "startRecognition: failure for wrong status");
            returnCode = RC_FAILURE;
            return returnCode;
        }

        SoundTrigger.KeyphraseRecognitionExtra[] stKpRecognitionExtraArray
                = extendedSmModel.getKeyphraseRecognitionExtra();
        if (null == stKpRecognitionExtraArray) {
            LogUtils.d(TAG, "startRecognition: failure for stKpRecognitionExtraArray");
            returnCode = RC_FAILURE;
            return returnCode;
        }

        boolean bCaptureRequest = SharedPreferenceUtils.getIsLABEnabled(mAppContext);
        boolean bAllowMultipleTriggers = false;

        // fill opaque data
        byte[] opaqueByteArray;
        OpaqueDataEncoder opaqueDataEncoder;
        int histBufferDuration = FileUtils.getHistBufferTimeSetting();
        int preRollDuration = FileUtils.getPreRollDurationSetting();
        ISmModel.MODEL_VERSION version = extendedSmModel.getSoundModelVersion();
        if (ISmModel.MODEL_VERSION.VERSION_3_0 == version) {
            boolean hasKeywordIndicesParam = false;
            boolean hasTimeStampParam = false;
            opaqueDataEncoder = new OpaqueDataEncoder(smName,
                    true, hasKeywordIndicesParam, hasTimeStampParam);
            int cnnKeyphraseLevel = FileUtils.getCNNKeyphraseConfidenceLevelSetting();
            int vopUserLevel = FileUtils.getVOPUserConfidenceLevelSetting();
            int gmmKeyphraseLevel = FileUtils.getKeyphraseConfidenceLevelSetting();
            int gmmUserLevel = FileUtils.getUserConfidenceLevelSetting();
            opaqueByteArray = opaqueDataEncoder.fillOpaqueDataByteBuffer(gmmKeyphraseLevel,
                    gmmUserLevel, cnnKeyphraseLevel, vopUserLevel,
                    histBufferDuration, preRollDuration, 0, 0,stKpRecognitionExtraArray);
        } else {
            opaqueDataEncoder = new OpaqueDataEncoder(smName, false, false, false);
            opaqueByteArray = opaqueDataEncoder.fillOpaqueDataByteBuffer(histBufferDuration,
                        preRollDuration);
        }

        SoundTrigger.RecognitionConfig stRecognitionConfig
                = new SoundTrigger.RecognitionConfig(bCaptureRequest, bAllowMultipleTriggers,
                stKpRecognitionExtraArray, opaqueByteArray);
        extendedSmModel.setRecognitionConfig(stRecognitionConfig);

        // call STM API
        UUID smUUID = extendedSmModel.getSoundModelUUID();
        returnCode = mSoundTriggerManager.startRecognition(smUUID, getPendingIntent(smName),
                stRecognitionConfig);
        if (RC_OK == returnCode) {
            LogUtils.d(TAG, "startRecognition: STM startRecognition success");
            extendedSmModel.setSessionStatus(IExtendedSmModel.SessionStatus.STARTED);
        }

        return returnCode;
    }

    private PendingIntent getPendingIntent(String smName) {
        Intent intent = new Intent();
        intent.setAction(ACTION_RECOGNITION);
        intent.setClass(mAppContext, SmartAssistantService.class);
        intent.putExtra(KEY_EXTRA_SM_NAME, smName);
        return PendingIntent.getService(mAppContext, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
    }

    private boolean isStateTransitional(IExtendedSmModel.SessionStatus fromState,
                                        IExtendedSmModel.SessionStatus toState) {
        switch (toState) {
            case UNLOADED:
                if (IExtendedSmModel.SessionStatus.LOADED == fromState
                        || IExtendedSmModel.SessionStatus.STOPPED == fromState) {
                    return true;
                }
                break;
            case LOADED:
                if (IExtendedSmModel.SessionStatus.UNLOADED == fromState) {
                    return true;
                }
                break;
            case STARTED:
                if (IExtendedSmModel.SessionStatus.LOADED == fromState
                        || IExtendedSmModel.SessionStatus.STOPPED == fromState) {
                    return true;
                }
                break;
            case STOPPED:
                if (IExtendedSmModel.SessionStatus.STARTED == fromState) {
                    return true;
                }
                break;
            default:
                LogUtils.e("isStateTransitional: unrecognized state" + toState);
                break;
        }

        return false;
    }

    private int stopRecognition(String smName) {
        int returnCode;
        LogUtils.d(TAG, "stopRecognition: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "stopRecognition: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        // judge the sm whether stopped or not
        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);
        if (IExtendedSmModel.SessionStatus.STOPPED == extendedSmModel.getSessionStatus()
                || IExtendedSmModel.SessionStatus.LOADED == extendedSmModel.getSessionStatus()) {
            LogUtils.d(TAG, "stopRecognition: already stopped, return success directly");
            returnCode = RC_OK;
            return returnCode;
        }

        boolean bCanTransit = isStateTransitional(extendedSmModel.getSessionStatus(),
                IExtendedSmModel.SessionStatus.STOPPED);
        if (!bCanTransit) {
            LogUtils.d(TAG, "stopRecognition: failure for wrong status");
            returnCode = RC_WRONG_STATUS;
            return returnCode;
        }

        // call STM API
        UUID smUUID = extendedSmModel.getSoundModelUUID();
        returnCode = mSoundTriggerManager.stopRecognition(smUUID);
        if (RC_OK == returnCode) {
            LogUtils.d(TAG, "stopRecognition: STM stopRecognition success");
            extendedSmModel.setSessionStatus(IExtendedSmModel.SessionStatus.STOPPED);
        }

        return returnCode;
    }

    private int unloadSM(String smName) {
        int returnCode;
        LogUtils.d(TAG, "unloadSM: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "unloadSM: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        // judge the sm whether unloaded or not
        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);
        if (IExtendedSmModel.SessionStatus.UNLOADED == extendedSmModel.getSessionStatus()) {
            LogUtils.d(TAG, "unloadSM: already unloaded, return success directly");
            returnCode = RC_OK;
            return returnCode;
        }

        boolean bCanTransit = isStateTransitional(extendedSmModel.getSessionStatus(),
                IExtendedSmModel.SessionStatus.UNLOADED);
        if (!bCanTransit) {
            LogUtils.d(TAG, "unloadSM: failure for wrong status");
            returnCode = RC_FAILURE;
            return returnCode;
        }

        // call STM API
        returnCode = mSoundTriggerManager.unloadSoundModel(extendedSmModel.getSoundModelUUID());
        if (RC_OK == returnCode) {
            LogUtils.d(TAG, "unloadSM: STM unloadSoundModel success");
            extendedSmModel.setSessionStatus(IExtendedSmModel.SessionStatus.UNLOADED);
        }

        return returnCode;
    }

    @Override
    public boolean isRecognitionActive(String smName) {
        UUID smUUID = mExtendedSmMgr.getSoundModel(smName).getSoundModelUUID();
        boolean bRecognitionActive = mSoundTriggerManager.isRecognitionActive(smUUID);
        LogUtils.d(TAG, "isRecognitionActive: bRecognitionActive = " + bRecognitionActive);
        return bRecognitionActive;
    }
}

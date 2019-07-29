/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.controller;

import android.content.Context;
import android.os.Environment;

import com.qualcomm.qti.sva.R;
import com.qualcomm.qti.sva.wrapper.SystemPropertiesWrapper;

public class Global {
    public final static String PROP_KEY_DEBUG_MODE = "persist.vendor.sva.debugMode";
    public final static String DEBUG_MODE_ENABLED = "1";
    public final static String DEBUG_MODE_DISABLED = "0";

    // define SVA app root directory and its subdirectory
    public final static String NAME_APP = "SVA";
    public final static String NAME_TRAINING_SUBDIR = "trainings";
    public final static String NAME_VOICE_REQUESTS_SUBDIR = "voiceRequests";
    public final static String SUFFIX_WAV_FILE = ".wav";
    public final static String NAME_LANGUAGE_MODEL = "default.lm";
    public final static String PATH_ROOT
            = Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + NAME_APP;
    public final static String PATH_TRAINING_RECORDINGS = Global.PATH_ROOT + "/"
            + NAME_TRAINING_SUBDIR;
    public final static String PATH_TRAINING_RECORDING_TEMP_FILE = PATH_TRAINING_RECORDINGS
            + "/" + "temp_recording.wav";
    public final static String PATH_VOICE_REQUESTS = Global.PATH_ROOT + "/"
            + NAME_VOICE_REQUESTS_SUBDIR;
    public final static String SUFFIX_DATA_FILE = ".data";

    private final String TAG = Global.class.getSimpleName();

    private AssetsFileMgr mAssetFileMgr;
    private RecordingsMgr mRecordingsMgr;
    private ExtendedSmMgr mExtendedSmMgr;

    private boolean mIsToggleXmlParam = false;

    private Global() {
        mAssetFileMgr = new AssetsFileMgr();
        mRecordingsMgr = new RecordingsMgr();
        mExtendedSmMgr = new ExtendedSmMgr();
    }

    public static Global getInstance() {
        return SingletonHolder.INSTANCE;
    }

    public static boolean isSvaDebugModeEnabled() {
        return DEBUG_MODE_ENABLED.equalsIgnoreCase(
                SystemPropertiesWrapper.get(PROP_KEY_DEBUG_MODE));
    }

    public static boolean getIsLegacySTModuleSessionImpl(Context context) {
        return context.getResources().getBoolean(R.bool.basedOn_legacy_STModuleAPI_session_impl);
    }

    public AssetsFileMgr getAssetFileMgr() {
        return mAssetFileMgr;
    }

    public RecordingsMgr getRecordingsMgr() {
        return mRecordingsMgr;
    }

    public ExtendedSmMgr getExtendedSmMgr() {
        return mExtendedSmMgr;
    }

    public boolean getIsToggleXmlParam() {
        return mIsToggleXmlParam;
    }

    public void setIsToggleXmlParam(boolean bToggle) {
        mIsToggleXmlParam = bToggle;
    }

    private static final class SingletonHolder {
        private static final Global INSTANCE = new Global();

        private SingletonHolder() {

        }
    }
}

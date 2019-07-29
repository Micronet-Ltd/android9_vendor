/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.backup;

import static com.android.backup.BackupUtils.BACKUP_TYPE_ALL;
import static com.android.backup.BackupUtils.BACKUP_TYPE_CONTACTS;
import static com.android.backup.BackupUtils.BACKUP_TYPE_MMS;
import static com.android.backup.BackupUtils.BACKUP_TYPE_SMS;
import static com.android.backup.BackupUtils.EVENT_BACKUP_RESULT;
import static com.android.backup.BackupUtils.EVENT_FILE_CREATE_ERR;
import static com.android.backup.BackupUtils.EVENT_INIT_PROGRESS_TITLE;
import static com.android.backup.BackupUtils.EVENT_SET_PROGRESS_VALUE;
import static com.android.backup.BackupUtils.PREFERENCE_BACKUP_DATA;
import static com.android.backup.BackupUtils.PREFERENCE_BACKUP_DATA_IS_SUCCESS;
import static com.android.backup.BackupUtils.PREFERENCE_BACKUP_DATA_RECENT_FILENAME;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import com.android.backup.ContactsContract.RawContacts;
import com.android.backup.vcard.VCardComposer;
import com.android.backup.vcard.VCardComposer.HandlerForOutputStream;
import com.android.backup.vcard.VCardConfig;

import android.app.ProgressDialog;
import android.app.Service;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import com.qualcomm.qti.backup_wrapper.FileUtilsWrapper;
import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class BackupService extends Service {

    private static final String TAG = "BackupService";
    private static boolean DBG = true;
    private static final String BACKUP_DATA = "/backup/Data";
    private static final String BACKUP_FOLDER = "/backup";
    private static final String DEFAULT_BACKUP_CONTACTS_DIR = "/Contact";
    private static final String DEFAULT_BACKUP_MMS_SMS_DIR = "/Mms";
    private static final String DEFAULT_BACKUP_SMS_DIR = "/Sms";

    private static final String BACKUP_EXTRA_DIR = "backup_dir";
    private static final String BACKUP_EXTRA_DATA_CHK = "data_checked";
    private static final String BACKUP_EXTRA_MSG_SLOT = "msg_slot";
    private static final String BACKUP_EXTRA_PATH = "backup_path";

    private static final int VCARD_TYPE = VCardConfig.VCARD_TYPE_DEFAULT;

    private static final int BACKUP_RESULT_FAIL = 0;
    private static final int BACKUP_RESULT_OK = 1;

    private String mBackupPath;
    private String mBackupDataDir;
    private String mBackupRootDir;

    private RemoteCallbackList<IBackupCallback> mCallbackList;
    private IBackupCallback mBackupCallback;

    private boolean mCancelBackupFlag = false;
    private String mCurrentDir = null;
    private int mStartId = 0;
    private BackupMmsOperation mBackupMessageOper = null;

    private IRemoteBackupService.Stub mRemoteBinder = new IRemoteBackupService.Stub() {

        @Override
        public void unregisterCallback(String packageName, IBackupCallback cb)
                throws RemoteException {
            Log.v(TAG, "BackupService unregisterCallback");
            if (cb != null) {
                mBackupCallback = null;
                if (mCallbackList != null) {
                    mCallbackList.unregister(cb);
                }
            }
        }

        @Override
        public void registerCallback(String packageName, IBackupCallback cb)
                throws RemoteException {
            Log.v(TAG, "BackupService registerCallback");
            if (cb != null) {
                mBackupCallback = cb;
                if (mCallbackList != null) {
                    mCallbackList.register(cb);
                }
            }
        }

        @Override
        public void init(String packageName) throws RemoteException {
            Log.v(TAG, "BackupService init: " + packageName);
        }

        @Override
        public void setCancelBackupFlag(boolean flag) throws RemoteException {
            mCancelBackupFlag = flag;
            if (flag && mBackupMessageOper != null) {
                 mBackupMessageOper.cancel();
            }
        }
    };

    public void sendBackupMessage(int what, int type, int result) {
        Log.v(TAG, "BackupService sendCallbackMessage what: " + what + " type: " + type
                + " result: " + result);
        if (mBackupCallback == null) {
            return;
        }
        mCallbackList.beginBroadcast();
        try {
            mBackupCallback.handleBackupMsg(what, type, result);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        mCallbackList.finishBroadcast();
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.v(TAG, "BackupService: onBind");
        return mRemoteBinder;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.v(TAG, "BackupService: onCreate");
        mCallbackList = new RemoteCallbackList<IBackupCallback>();
        mCancelBackupFlag = false;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.v(TAG, "BackupService: onDestroy");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.v(TAG, "BackupService: onStartCommand: " + startId);
        mStartId = startId;
        mCancelBackupFlag = false;

        if(intent == null){
            SharedPreferences msp = getSharedPreferences(PREFERENCE_BACKUP_DATA, Context.MODE_PRIVATE);
            String name = msp.getString(PREFERENCE_BACKUP_DATA_RECENT_FILENAME, null);
            if(name != null && !msp.getBoolean(PREFERENCE_BACKUP_DATA_IS_SUCCESS, false)) {
                BackupUtils.deleteDir(new File(name));
            }
            return Service.START_NOT_STICKY;
        }

        mCurrentDir = intent.getStringExtra(BACKUP_EXTRA_DIR);
        boolean[] dataChecked = intent.getBooleanArrayExtra(BACKUP_EXTRA_DATA_CHK);
        int[] msgSlot = intent.getIntArrayExtra(BACKUP_EXTRA_MSG_SLOT);
        mBackupPath = intent.getStringExtra(BACKUP_EXTRA_PATH);
        mBackupDataDir = mBackupPath + BACKUP_DATA;
        mBackupRootDir = mBackupPath + BACKUP_FOLDER;

        BackupServiceThread backupThread = new BackupServiceThread(dataChecked, msgSlot);
        backupThread.start();

        return 0;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        return super.onUnbind(intent);
    }

    public boolean getCancelBackupFlag() {
        return mCancelBackupFlag;
    }

    public void setCancelBackupFlag(boolean flag) {
        mCancelBackupFlag = flag;
    }

    private void outputRawContactsAsVCardFile(Uri uri, String filename,
            String selection, String[] selectionArgs) {

        OutputStream outputStream = null;
        try {
            outputStream = new FileOutputStream(filename);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return;
        }

        final VCardComposer composer = new VCardComposer(BackupService.this,
                VCARD_TYPE, false);
        composer.addHandler(composer.new HandlerForOutputStream(outputStream));

        // No extra checks since composer always uses restricted views
        if (!composer.init(uri, selection, selectionArgs, null)){
            Log.w(TAG, " VCardComposer init error! ");
            return;
        }

        // Update the progressbar info
        int max = composer.getCount();
        Log.d(TAG, "outputRawContactsAsVCardFile max: " + max);
        sendBackupMessage(EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_CONTACTS, max);

        int i = 1;
        while (!composer.isAfterLast()) {
            Log.d(TAG, "OutputRawContactsAsVCardFile : i =" + i);
            if (mCancelBackupFlag) {
                return;
            }
            try {
                if (BackupUtils.isInternalPath(mBackupPath)) {
                    if (BackupUtils.isInternalFull()) {
                        mCancelBackupFlag = true;
                        return;
                    }
                }
            } catch (IllegalArgumentException e) {
                Log.v(TAG, "Backup contacts: " + e.getMessage());
                return;
            }
            if (!composer.createOneEntry()) {
                Log.w(TAG, "Failed to output a contact.");
            }

            if (i % 50 == 0) {
                sendBackupMessage(EVENT_SET_PROGRESS_VALUE, i, 0);
            }
            i++;
        }
        composer.terminate();
    }

    public boolean backupContacts() {
        String filename = mBackupDataDir + mCurrentDir
                + DEFAULT_BACKUP_CONTACTS_DIR + "/contact.vcf";
        String selection = "deleted=0";
        outputRawContactsAsVCardFile(RawContacts.CONTENT_URI, filename,
                selection, null);
        return new File(filename).exists();

    }

    private boolean backupMMS(String dir, int slot) {
        mBackupMessageOper = new BackupMmsOperation(this, null, dir, slot, mBackupPath);
        mBackupMessageOper.backupMmsMessages();
        mBackupMessageOper = null;
        return new File(dir).exists();
    }

    private boolean backupSMS(String dir, int slot) {
        mBackupMessageOper = new BackupMmsOperation(this, null, dir, slot, mBackupPath);
        mBackupMessageOper.backupSmsMessages();
        mBackupMessageOper = null;
        return new File(dir).exists();
    }

    private class BackupServiceThread extends Thread {

        private boolean[] mDataChecked;
        private int[] mMsgSlot;

        public BackupServiceThread(boolean[] dataChecked, int[] msgSlot) {
            super("OperateThread1");
            mDataChecked = dataChecked;
            mMsgSlot = msgSlot;
        }

        public void run() {
            Log.v(TAG, "BackupService: BackupServiceThread");
            Log.d(TAG, "BackupServiceThread directory: " + mCurrentDir);

            SharedPreferences.Editor editor = getSharedPreferences(PREFERENCE_BACKUP_DATA, Context.MODE_PRIVATE).edit();
            editor.putString(PREFERENCE_BACKUP_DATA_RECENT_FILENAME, mBackupDataDir + mCurrentDir).commit();
            editor.putBoolean(PREFERENCE_BACKUP_DATA_IS_SUCCESS, false).commit();

            int num = 1;

            // Backup contacts
            boolean contactFail = false;
            if (!mCancelBackupFlag && mDataChecked[0]) {
                File contacts_dir = new File(mBackupDataDir
                        + mCurrentDir + DEFAULT_BACKUP_CONTACTS_DIR);
                if (prepareBackupDir(contacts_dir)) {
                    backupContacts();
                } else {
                    contactFail = true;
                    Log.d(TAG, "OperateThread: contacts error");
                }
                sendBackupMessage(EVENT_SET_PROGRESS_VALUE, num, 0);
                num++;
            }

            // Backup MMS
            boolean mmsFail = false;
            if (!mCancelBackupFlag && mDataChecked[1]) {
                String mms_file = mBackupDataDir + mCurrentDir
                        + DEFAULT_BACKUP_MMS_SMS_DIR;
                File mms_dir = new File(mms_file);
                if (prepareBackupDir(mms_dir)) {
                    mmsFail = !backupMMS(mms_file, mMsgSlot[0]);
                } else {
                    mmsFail = true;
                    Log.d(TAG, "OperateThread:mms error");
                }
                if (DBG) {
                    Log.d(TAG, "Backup MMS:mmsFail:" + mmsFail);
                    Log.d(TAG, "Backup MMS:mCurrentDir:" + mCurrentDir);
                }
                if (!mmsFail && BackupUtils.isMultiSimEnabled()) {
                    BackupUtils.setBackUpMmsSmsSlot(BackupService.this.getApplicationContext(),
                            mCurrentDir + "_Mms", mMsgSlot[0]);
                }
                sendBackupMessage(EVENT_SET_PROGRESS_VALUE, num, 0);
                num++;
            }

            // Backup SMS
            boolean smsFail = false;
            if (!mCancelBackupFlag && mDataChecked[2]) {
                String mms_sms_file = mBackupDataDir + mCurrentDir
                        + DEFAULT_BACKUP_SMS_DIR;
                File mms_sms_dir = new File(mms_sms_file);

                if (prepareBackupDir(mms_sms_dir)) {
                    smsFail = !backupSMS(mms_sms_file, mMsgSlot[1]);// wrong
                } else {
                    smsFail = true;
                    Log.d(TAG, "OperateThread:sms error");
                }
                if (DBG) {
                    Log.d(TAG, "Backup SMS:smsFail:" + smsFail);
                    Log.d(TAG, "Backup SMS:mCurrentDir:" + mCurrentDir);
                }
                if (!smsFail && BackupUtils.isMultiSimEnabled()) {
                    BackupUtils.setBackUpMmsSmsSlot(BackupService.this.getApplicationContext(),
                            mCurrentDir + "_Sms", mMsgSlot[1]);
                }
                sendBackupMessage(EVENT_SET_PROGRESS_VALUE, num, 0);
                num++;
            }

            if (DBG) {
                Log.d(TAG, "contactFail:" + contactFail);
                Log.d(TAG, "mmsFail:" + mmsFail);
                Log.d(TAG, "smsFail:" + smsFail);
            }

            if (!contactFail && !mmsFail && !smsFail) {
                if (mDataChecked[0] || mDataChecked[1] || mDataChecked[2])
                    sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_ALL, BACKUP_RESULT_OK);
                else if (mDataChecked[0])
                    sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_CONTACTS, BACKUP_RESULT_OK);
                else if (mDataChecked[1])
                    sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_MMS, BACKUP_RESULT_OK);
                else if (mDataChecked[2])
                    sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_SMS, BACKUP_RESULT_OK);

//                Intent scanIntent =
//                        new Intent("com.android.fileexplorer.action.MEDIA_SCANNER_SCAN_ALL");
//                scanIntent.setData(Uri.fromFile(new File(BackupUtils.
//                        getDefaultInternalPath(BackupService.this.getApplicationContext()))));
//                sendBroadcast(scanIntent);
            } else {
                sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_ALL, BACKUP_RESULT_FAIL);
            }

            editor.putBoolean(PREFERENCE_BACKUP_DATA_IS_SUCCESS, true).commit();

            BackupService.this.stopSelf(mStartId);
        }
    }

    private boolean prepareBackupDir(File dir) {
        if (!dir.exists()) {
            Log.d(TAG, "prepareBackupDir backup dir is not exist");
            if (!dir.mkdir()) {
                if (DBG)
                    Log.d(TAG,
                            "prepareBackupDir backup stop - can't create base directory "
                                    + dir.getPath());
                sendBackupMessage(EVENT_FILE_CREATE_ERR, 0, 0);
                return false;
            } else {
                Log.d(TAG, "prepareBackupDir change file permission mode");
              /*  FileUtils.setPermissions(mBackupRootDir,
                        FileUtils.S_IRWXO | FileUtils.S_IRWXU
                                | FileUtils.S_IRWXG, -1, -1);
                return true;*/
                try {
                    FileUtilsWrapper.setPermissions(mBackupRootDir,
                        FileUtilsWrapper.S_IRWXO | FileUtilsWrapper.S_IRWXU
                                | FileUtilsWrapper.S_IRWXG, -1, -1);
                    return true;
                } catch (WrapperNotSupportException wrapperE) {
                    Log.e(TAG, wrapperE.getMessage());
                    return false;
                }
            }
        }
        return true;
    }
}

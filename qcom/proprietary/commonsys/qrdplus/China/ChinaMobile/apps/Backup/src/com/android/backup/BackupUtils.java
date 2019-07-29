/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.backup;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.StatFs;
import android.provider.Settings;
import android.os.storage.StorageManager;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import com.qualcomm.qti.backup_wrapper.PhoneConstantsWrapper;
import com.qualcomm.qti.backup_wrapper.StorageManagerWrapper;
import com.qualcomm.qti.backup_wrapper.StorageVolumeWrapper;
import com.qualcomm.qti.backup_wrapper.TelephonyManagerWrapper;
import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class BackupUtils {

    private static final String TAG = "BackupUtils";
    public static String MULTI_SIM_NAME = "perferred_name_sub";
    public static final int SLOT_ALL = 2;
    public static final int SLOT_1 = 0;
    public static final int SLOT_2 = 1;
    public static final int EVENT_STOP_BACKUP = 100;
    public static final int EVENT_FILE_CREATE_ERR = 101;
    public static final int EVENT_INIT_PROGRESS_TITLE = 102;
    public static final int EVENT_SET_PROGRESS_VALUE = 103;
    public static final int EVENT_SDCARD_NO_SPACE = 105;
    public static final int EVENT_RESUME_BACKUP_THREAD = 106;
    public static final int EVENT_BACKUP_RESULT = 107;
    public static final int EVENT_RESTORE_RESULT = 108;
    public static final int EVENT_SDCARD_FULL = 109;

    public static final int BACKUP_TYPE_CONTACTS = 1;
    public static final int BACKUP_TYPE_MMS = 2;
    public static final int BACKUP_TYPE_SMS = 3;
    public static final int BACKUP_TYPE_ALL = 4;

    public static final String LOCATION_PREFERENCE_NAME = "Location_Preference";
    public static final String BACKUP_ITEMS_PREFERENCE_NAME = "Backup_Items_Preference";
    public static final String KEY_BACKUP_LOCATION = "Key_Backup_Location";
    public static final String KEY_RESTORE_LOCATION = "Key_Restore_Location";

    public static final String PREFERENCE_BACKUP_DATA = "Preference_Backup_Data";
    public static final String PREFERENCE_BACKUP_DATA_RECENT_FILENAME = "Recent_FileName";
    public static final String PREFERENCE_BACKUP_DATA_IS_SUCCESS = "Is_Success";

    /**
     * Return the sim name of subscription.
     */
    public static String getMultiSimName(Context context, int subscription) {
        try {
            if (subscription >= TelephonyManagerWrapper.getDefault().getPhoneCount() || subscription < 0) {
                return null;
            }
        } catch (WrapperNotSupportException wrapperE) {
            Log.e(TAG, wrapperE.getMessage());
            return null;
        }
        String multiSimName = Settings.System.getString(context.getContentResolver(),
                MULTI_SIM_NAME + (subscription + 1));
        if (multiSimName == null) {
            if (subscription == PhoneConstantsWrapper.SUB1) {
                return context.getString(R.string.slot1);
            } else if (subscription == PhoneConstantsWrapper.SUB2) {
                return context.getString(R.string.slot2);
            }
        }
        return multiSimName;
    }

    /**
     * Decide whether the current product  is DSDS in MMS
     */
    public static boolean isMultiSimEnabled() {
        try {
            return TelephonyManagerWrapper.getDefault().isMultiSimEnabled();
        } catch (WrapperNotSupportException wrapperE) {
            Log.e(TAG, wrapperE.getMessage());
            return false;
        }
        
    }

    public static int getBackUpMmsSmsSlot(Context context, String key) {
        SharedPreferences prefs = context.getSharedPreferences(
                BACKUP_ITEMS_PREFERENCE_NAME, Context.MODE_MULTI_PROCESS);
        return prefs.getInt(key, SLOT_ALL);
    }

    public static void setBackUpMmsSmsSlot(Context context, String key, int slot) {
        SharedPreferences.Editor editPrefs = context.getSharedPreferences(
                BACKUP_ITEMS_PREFERENCE_NAME, Context.MODE_MULTI_PROCESS).edit();
        editPrefs.putInt(key, slot).commit();
    }

    public static String getSDPath(Context context) {
    //    IMountService ms;
        StorageVolumeWrapper[] volumes;
        StorageManager sm = null;
        StorageManagerWrapper smw = null;
        try {
           // sm = StorageManagerWrapper.from(context);
            smw = new StorageManagerWrapper(StorageManagerWrapper.from(context));
        } catch (WrapperNotSupportException wrapperE) {
            Log.e(TAG, wrapperE.getMessage());
            return null;
        }
        
        try {
            //ms = IMountService.Stub.asInterface(ServiceManager.getService("mount"));
            volumes = smw.getVolumeList();
        } catch (Exception e) {
            return null;
        }
        try {
            for (int i = 0; i < volumes.length; i++) {
                if (volumes[i].isRemovable() && volumes[i].allowMassStorage()
                        && volumes[i].getPath().toUpperCase().contains("SDCARD")) {
                    return volumes[i].getPath();
                }
            }
        } catch (WrapperNotSupportException wrapperE) {
            Log.e(TAG, wrapperE.getMessage());
        }
        return null;
    }

    public static boolean isInternalFull() throws IllegalArgumentException
    {
        long available = getInternalAvailableSpace();
        if (available < 1 * 1024 * 1024) {
            return true;
        }
        return false;
    }

    private static long getInternalAvailableSpace() throws IllegalArgumentException
    {
        StatFs stat = new StatFs(Environment.getExternalStorageDirectory().getAbsolutePath());
        long remaining = (long) stat.getAvailableBlocks() * (long) stat.getBlockSize();
        return remaining;
    }

    public static String getDefaultInternalPath(Context context){
        File[] dirs = context.getExternalFilesDirs(null);
        if (dirs != null && dirs.length > 0){
            return dirs[0].getAbsolutePath();
        }
        return null;
    }

    public static String getBackupPath(Context context) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(
                LOCATION_PREFERENCE_NAME, Context.MODE_PRIVATE);
        String backupPath = sharedPreferences.getString(KEY_BACKUP_LOCATION,
                getDefaultInternalPath(context));
        return backupPath;
    }

    public static boolean isDirAvalible(String path) {
        File dir = new File(path);
        if (!dir.exists() || !dir.isDirectory()) {
            return false;
        }
        StatFs stat = new StatFs(path);
        if (stat.getBlockCount() > 0)
            return true;
        else
            return false;
    }

    public static String getRestorePath(Context context) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(
                LOCATION_PREFERENCE_NAME, Context.MODE_PRIVATE);
        String restorePath = sharedPreferences.getString(KEY_RESTORE_LOCATION,
                getDefaultInternalPath(context));
        return restorePath;
    }

    /**
     * Returns true/false, if the provided path is internal path
     */
    public static boolean isInternalPath(String path){
        try {
            File file = new File(path);
            String canonicalPath = file.getCanonicalPath();
            String canonicalExternal = Environment.getExternalStorageDirectory()
                    .getCanonicalPath();
            if (canonicalPath.startsWith(canonicalExternal)){
                return true;
            }
        } catch (IOException e){
            e.printStackTrace();
        }
        return false;
    }

    public static void deleteDir(File dir) {
        if (dir == null || !dir.exists()) {
            return;
        }
        if (!dir.isDirectory()) {
            dir.delete();
            return;
        }
        for (File files : dir.listFiles()) {
            if (files.isFile()) {
                files.delete();
            } else if (files.isDirectory()) {
                deleteDir(files);
            }
        }
        dir.delete();
    }
}

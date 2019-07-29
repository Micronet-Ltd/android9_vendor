/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.os.FileUtils;
import android.util.Log;

import java.io.File;

public class FileUtilsWrapper {

    private static final String TAG = "FileUtilsWrapper";

    public static final int S_IRWXU = FileUtils.S_IRWXU;
    public static final int S_IRWXG = FileUtils.S_IRWXG;
    public static final int S_IRWXO = FileUtils.S_IRWXO;

    /**
     * Set owner and mode of of given {@link File}.
     *
     * @param mode to apply through {@code chmod}
     * @param uid to apply through {@code chown}, or -1 to leave unchanged
     * @param gid to apply through {@code chown}, or -1 to leave unchanged
     * @return 0 on success, otherwise errno.
     */
    public static int setPermissions(String path, int mode, int uid, int gid) throws WrapperNotSupportException{
        try{
            return FileUtils.setPermissions(path, mode, uid, gid);
        } catch (Exception e) {
            Log.e(TAG,"myUserId: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
        
    }
    
}
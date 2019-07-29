/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.text.TextUtils;
import android.util.Log;

import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class TextUtilsWrapper {

    private static final String TAG = "TextUtilsWrapper";

    public static boolean isPrintableAsciiOnly(final CharSequence str) throws WrapperNotSupportException {
        try {
            return TextUtils.isPrintableAsciiOnly(str);
        } catch (Exception e) {
            Log.e(TAG,"isPrintableAsciiOnly: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }
}
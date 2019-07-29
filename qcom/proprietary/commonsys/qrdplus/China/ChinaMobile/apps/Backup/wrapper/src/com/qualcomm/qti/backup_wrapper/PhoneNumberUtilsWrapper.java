/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.telephony.PhoneNumberUtils;
import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class PhoneNumberUtilsWrapper {
    
    private static final String TAG = "PhoneNumberUtilsWrapper";
    
    public static String formatNumber(String source, int defaultFormattingType) throws WrapperNotSupportException {
        try {
            return PhoneNumberUtils.formatNumber(source, defaultFormattingType);
        } catch (Exception e) {
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }
}
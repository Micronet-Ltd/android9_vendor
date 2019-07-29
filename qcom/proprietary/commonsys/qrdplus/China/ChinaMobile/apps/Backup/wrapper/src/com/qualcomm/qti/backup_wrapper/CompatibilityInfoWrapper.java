/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.content.res.CompatibilityInfo;

public class CompatibilityInfoWrapper {

    private static final String TAG = "CompatibilityInfoWrapper";

    private CompatibilityInfo mCompatibilityInfo;
    public final float applicationScale = mCompatibilityInfo.applicationScale;

    public CompatibilityInfoWrapper(CompatibilityInfo instance) {
        mCompatibilityInfo = instance;
    }
}
/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch.switchaction;

import android.content.Context;
import android.content.pm.PackageManager;

import com.qualcomm.qti.carrierswitch.AbstractSwitcher;
import com.qualcomm.qti.carrierswitch.LogUtil;
import com.qualcomm.qti.carrierswitch.R;

import java.util.ArrayList;
import java.util.List;


public class ApplicationSwitcher extends AbstractSwitcher {

    private Context mContext;
    private List<ParsedInfo> mApplications = new ArrayList<>();

    public ApplicationSwitcher(Context context) {
        mContext = context;
        try {
            mApplications = CarrierXMLParser.parseFrom(
                    context, R.xml.applications);
            for (ParsedInfo info: mApplications) {
                LogUtil.v(this, "ApplicationSwitcher mApplications: " + info);
            }
        } catch (Exception e) {
            LogUtil.e(this, "ApplicationSwitcher init e =" + e);
        }
    }

    @Override
    public boolean executeNow(int oldCarrier, int newCarrier) {
        LogUtil.d(this, "executeNow");
        boolean ret = true;
        for (ParsedInfo info : mApplications) {
            if (info instanceof ApplicationParsedInfo) {
                ApplicationParsedInfo app = (ApplicationParsedInfo)info;
                try {
                    if (app.carrierId == oldCarrier) {
                        LogUtil.d(this, "executeNow disable app: " + app.packageName);
                        PackageManager pm = mContext.getPackageManager();
                        pm.setApplicationEnabledSetting(app.packageName,
                                PackageManager.COMPONENT_ENABLED_STATE_DISABLED, 0);

                    }
                    if (app.carrierId == newCarrier) {
                        LogUtil.d(this, "executeNow enable app: " + app.packageName);
                        PackageManager pm = mContext.getPackageManager();
                        pm.setApplicationEnabledSetting(app.packageName,
                                PackageManager.COMPONENT_ENABLED_STATE_ENABLED, 0);

                    }
                } catch (Exception e) {
                    LogUtil.e(this, "executeNow error: " +  e.getMessage());
                    ret = false;
                }
            }
        }
        return ret;
    }
}

/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch.switchaction;

import android.app.ActivityManager;
import android.content.Context;
import android.content.om.IOverlayManager;
import android.os.ServiceManager;

import com.qualcomm.qti.carrierswitch.AbstractSwitcher;

import java.util.ArrayList;
import java.util.List;

import com.qualcomm.qti.carrierswitch.LogUtil;
import com.qualcomm.qti.carrierswitch.R;

public class OverlaySwitcher extends AbstractSwitcher {

    private List<ParsedInfo> mOverlays = new ArrayList<>();


    public OverlaySwitcher(Context context) {
        try {
            mOverlays = CarrierXMLParser.parseFrom(
                    context, R.xml.overlays);
            for (ParsedInfo info: mOverlays) {
                LogUtil.v(this, "OverlaySwitcher mCMCCOverlays: " + info);
            }
        } catch (Exception e) {
            LogUtil.e(this, "OverlaySwitcher init e =" + e);
        }
    }

    @Override
    public boolean executeNow(int oldCarrier, int newCarrier) {
        LogUtil.d(this, "executeNow");
        boolean ret = true;
        for (ParsedInfo info : mOverlays) {
            if (info instanceof OverlayParsedInfo) {
                OverlayParsedInfo overlay = (OverlayParsedInfo)info;
                try {
                    if (overlay.carrierId == oldCarrier) {
                        LogUtil.d(this, "executeNow disable overlay: " + overlay.packageName
                                + ", user: " + ActivityManager.getCurrentUser());
                        IOverlayManager om = IOverlayManager.Stub.asInterface(
                                ServiceManager.getService(Context.OVERLAY_SERVICE));
                        om.setEnabled(overlay.packageName, false,
                                ActivityManager.getCurrentUser());
                    }
                    if (overlay.carrierId == newCarrier) {
                        LogUtil.d(this, "executeNow enable overlay: " + overlay.packageName
                                + ", user: " + ActivityManager.getCurrentUser());
                        IOverlayManager om = IOverlayManager.Stub.asInterface(
                                ServiceManager.getService(Context.OVERLAY_SERVICE));
                        om.setEnabled(overlay.packageName, true,
                                ActivityManager.getCurrentUser());
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
